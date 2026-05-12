import ctypes
import json
import os
import random
from http.server import SimpleHTTPRequestHandler, HTTPServer

class Pozitie(ctypes.Structure):
    _fields_ = [("linie", ctypes.c_int), ("coloana", ctypes.c_int)]

class Mutare(ctypes.Structure):
    _fields_ = [("sursa", Pozitie), ("destinatie", Pozitie), 
                ("piesa_mutata", ctypes.c_char), ("piesa_capturata", ctypes.c_char), 
                ("promovare", ctypes.c_char)]

lib_path = os.path.join(os.path.dirname(__file__), 'libchess.so')
try:
    lib = ctypes.CDLL(lib_path)
except OSError:
    print("Nu am putut incarca libchess.so! Ruleaza 'make' intai.")
    exit(1)

TablaType = (ctypes.c_char * 8) * 8
lib.initializare_stare_reguli.argtypes = []

lib.get_stare_rocada.argtypes = [ctypes.POINTER(ctypes.c_bool)]
lib.set_stare_rocada.argtypes = [ctypes.POINTER(ctypes.c_bool)]
lib.initializare_tabla.argtypes = [TablaType]
lib.mutare_valida.argtypes = [TablaType, Pozitie, Pozitie, ctypes.c_int, ctypes.POINTER(Mutare)]
lib.mutare_valida.restype = ctypes.c_bool
lib.executa_mutare.argtypes = [TablaType, Mutare]
lib.este_regele_in_sah.argtypes = [TablaType, ctypes.c_int]
lib.este_regele_in_sah.restype = ctypes.c_bool
lib.verificare_sah_mat.argtypes = [TablaType, ctypes.c_int]
lib.verificare_sah_mat.restype = ctypes.c_bool

current_board = TablaType()
current_turn = 0 # 0 = ALB, 1 = NEGRU
game_over = False
message = ""
game_mode = "local" # 'local', 'white', 'black'

def check_insufficient_material():
    white_pieces = []
    black_pieces = []
    
    for r in range(8):
        for c in range(8):
            p = current_board[r][c].decode('utf-8')
            if p == ' ': continue
            if p in ('K', 'k'): continue
            
            if p.isupper():
                white_pieces.append((p, r, c))
            else:
                black_pieces.append((p, r, c))
                
    # Daca exista orice alta piesa decat N, B, nu e remiza
    for p, r, c in white_pieces + black_pieces:
        if p.lower() not in ('n', 'b'):
            return False
            
    nw = len(white_pieces)
    nb = len(black_pieces)
    
    # 1. Rege vs rege
    if nw == 0 and nb == 0: return True
    
    # 2 & 3. Rege+cal/nebun vs Rege
    if nw == 1 and nb == 0: return True
    if nw == 0 and nb == 1: return True
    
    # 4, 5, 6, 7. Ambele au cate o piesa usoara
    if nw == 1 and nb == 1:
        p_w, r_w, c_w = white_pieces[0]
        p_b, r_b, c_b = black_pieces[0]
        
        # Daca macar unul e cal
        if p_w.lower() == 'n' or p_b.lower() == 'n':
            return True
            
        # Rege+nebun vs Rege+nebun (doar daca sunt pe aceeasi culoare)
        if p_w.lower() == 'b' and p_b.lower() == 'b':
            culoare_w = (r_w + c_w) % 2
            culoare_b = (r_b + c_b) % 2
            if culoare_w == culoare_b:
                return True
                
    return False

def has_valid_moves_4x4(board, turn):
    for l_s in range(4):
        for c_s in range(4):
            if is_piece_of_player(board[l_s][c_s], turn):
                sursa = Pozitie(l_s, c_s)
                for l_d in range(4):
                    for c_d in range(4):
                        dest = Pozitie(l_d, c_d)
                        m = Mutare()
                        if lib.mutare_valida(board, sursa, dest, turn, ctypes.byref(m)):
                            return True
    return False

def check_game_state():
    global game_over, message
    
    if check_insufficient_material():
        game_over = True
        message = "REMIZA! Material insuficient pentru sah mat."
        return

    adversar_nume = "ALB" if current_turn == 0 else "NEGRU"
    in_sah = lib.este_regele_in_sah(current_board, current_turn)
    
    if game_mode == 'training':
        has_moves = has_valid_moves_4x4(current_board, current_turn)
    else:
        has_moves = not lib.verificare_sah_mat(current_board, current_turn)
        
    if in_sah:
        if not has_moves:
            game_over = True
            castigator = "NEGRU" if current_turn == 0 else "ALB"
            message = f"SAH MAT! A castigat jucatorul {castigator}."
        else:
            message = f"SAH! Tura jucatorului {adversar_nume}."
    else:
        if not has_moves:
            game_over = True
            message = "REMIZA! Nicio mutare posibila (Pat)."
        else:
            message = f"Tura jucatorului {adversar_nume}."

def is_piece_of_player(p, turn):
    if p == b' ': return False
    if turn == 0: return p.isupper()
    return p.islower()

def copy_board(src):
    dst = TablaType()
    for i in range(8):
        for j in range(8):
            dst[i][j] = src[i][j]
    return dst

def find_mate(board, turn, depth):
    if lib.este_regele_in_sah(board, turn) and lib.verificare_sah_mat(board, turn):
        return (None, turn == 1)
    if depth == 0: return (None, False)
    
    valid_moves = []
    for l_s in range(4):
        for c_s in range(4):
            if is_piece_of_player(board[l_s][c_s], turn):
                sursa = Pozitie(l_s, c_s)
                for l_d in range(4):
                    for c_d in range(4):
                        dest = Pozitie(l_d, c_d)
                        m = Mutare()
                        if lib.mutare_valida(board, sursa, dest, turn, ctypes.byref(m)):
                            valid_moves.append(m)
    if not valid_moves: return (None, False)
    
    if turn == 0:
        best_m = valid_moves[0]
        for m in valid_moves:
            nb = copy_board(board)
            lib.executa_mutare(nb, m)
            _, is_mate = find_mate(nb, 1, depth - 1)
            if is_mate: return (m, True)
        return (best_m, False)
    else:
        for m in valid_moves:
            nb = copy_board(board)
            lib.executa_mutare(nb, m)
            _, is_mate = find_mate(nb, 0, depth - 1)
            if not is_mate: return (None, False)
        return (None, True)

def record_move(board, sursa, dest, piesa_capturata):
    piece = board[sursa.linie][sursa.coloana].decode('utf-8')
    if piece.lower() == 'p': piece = ''
    else: piece = piece.upper()
    
    file_s = chr(ord('a') + sursa.coloana)
    rank_s = str(8 - sursa.linie)
    file_d = chr(ord('a') + dest.coloana)
    rank_d = str(8 - dest.linie)
    is_cap = piesa_capturata != b' '
    move_str = f"{piece}{file_s}{rank_s}{'x' if is_cap else '-'}{file_d}{rank_d}"
    move_history.append(move_str)

def play_training_ai_move():
    global current_turn, game_over, message, training_moves_made, training_total_moves
    rem = training_total_moves - training_moves_made
    if rem <= 0: return
    depth = min(5, rem * 2 - 1)
    
    best_m, is_mate = find_mate(current_board, current_turn, depth)
    if is_mate and best_m:
        record_move(current_board, best_m.sursa, best_m.destinatie, best_m.piesa_capturata)
        lib.executa_mutare(current_board, best_m)
        if current_turn == 0:
            training_moves_made += 1
        current_turn = 1 - current_turn
        check_game_state()
    else:
        play_ai_move()

def play_ai_move():
    global current_turn, game_over
    if game_over: return
    
    valid_moves = []
    # Parcurgem tabla pt a gasi mutarile AI-ului
    for l_s in range(8):
        for c_s in range(8):
            piesa = current_board[l_s][c_s]
            if piesa == b' ': continue
            
            sursa = Pozitie(l_s, c_s)
            for l_d in range(8):
                for c_d in range(8):
                    if game_mode == 'training' and (l_d > 3 or c_d > 3 or l_s > 3 or c_s > 3):
                        continue
                        
                    dest = Pozitie(l_d, c_d)
                    mutare = Mutare()
                    if lib.mutare_valida(current_board, sursa, dest, current_turn, ctypes.byref(mutare)):
                        # Evalueaza mutarea
                        score = 0
                        cap = mutare.piesa_capturata.decode('utf-8').lower()
                        if cap == 'q': score = 9
                        elif cap == 'r': score = 5
                        elif cap in ('b', 'n'): score = 3
                        elif cap == 'p': score = 1
                        
                        valid_moves.append((mutare, score))
                        
    if valid_moves:
        valid_moves.sort(key=lambda x: x[1], reverse=True)
        best_score = valid_moves[0][1]
        best_moves = [m for m, s in valid_moves if s == best_score]
        chosen_move = random.choice(best_moves)
        
        record_move(current_board, chosen_move.sursa, chosen_move.destinatie, chosen_move.piesa_capturata)
        lib.executa_mutare(current_board, chosen_move)
        current_turn = 1 - current_turn
        check_game_state()

training_initial_board = TablaType()
training_moves_made = 0
training_total_moves = 4

def restore_training_board():
    global current_turn, game_over, message, training_moves_made
    for i in range(8):
        for j in range(8):
            current_board[i][j] = training_initial_board[i][j]
    current_turn = 0
    game_over = False
    message = "ANTRENAMENT: Ai depasit limita de mutari! Incearca din nou."
    training_moves_made = 0
    lib.initializare_stare_reguli()

def generate_training_board():
    global training_moves_made
    piese_disponibile = [b'Q', b'R', b'B', b'N']
    while True:
        # Golim tabla
        for i in range(8):
            for j in range(8):
                current_board[i][j] = b' '
                
        # Regele Alb (in careul 4x4)
        kw_r, kw_c = random.randint(0, 3), random.randint(0, 3)
        # Regele Negru
        while True:
            kb_r, kb_c = random.randint(0, 3), random.randint(0, 3)
            if abs(kw_r - kb_r) > 1 or abs(kw_c - kb_c) > 1:
                break
                
        current_board[kw_r][kw_c] = b'K'
        current_board[kb_r][kb_c] = b'k'
        
        piese = [random.choice([b'Q', b'R']), random.choice([b'Q', b'R', b'B', b'N'])]
        for p in piese:
            while True:
                r, c = random.randint(0, 3), random.randint(0, 3)
                if current_board[r][c] == b' ':
                    current_board[r][c] = p
                    break
                    
        # Daca Regele Negru nu este in sah la start, generarea e ok
        if not lib.este_regele_in_sah(current_board, 1):
            if not check_insufficient_material():
                _, mate_in_1 = find_mate(current_board, 0, 1)
                if mate_in_1:
                    continue # E prea simplu, vrem intre 2 si 7
                    
                solvable = False
                for m in range(2, 8):
                    _, is_mate = find_mate(current_board, 0, m * 2 - 1)
                    if is_mate:
                        global training_total_moves
                        training_total_moves = m
                        solvable = True
                        break
                if solvable:
                    break
                
    # Salvam starea initiala
    for i in range(8):
        for j in range(8):
            training_initial_board[i][j] = current_board[i][j]
    training_moves_made = 0

move_history = []

def init_game(mode="local"):
    global current_turn, game_over, message, game_mode, move_history
    game_mode = mode
    current_turn = 0
    game_over = False
    move_history = []
    
    if mode == 'training':
        message = "ANTRENAMENT: Da sah mat!"
    else:
        message = "Tura jucatorului ALB."
        
    lib.initializare_stare_reguli()
    
    if mode == 'training':
        generate_training_board()
    else:
        lib.initializare_tabla(current_board)
    
    if game_mode == 'black':
        # AI muta primul (cu Albul)
        play_ai_move()

init_game()

def board_to_list():
    return [[current_board[i][j].decode('utf-8') for j in range(8)] for i in range(8)]

class ChessHandler(SimpleHTTPRequestHandler):
    def end_headers(self):
        self.send_header('Cache-Control', 'no-cache, no-store, must-revalidate')
        super().end_headers()

    def do_GET(self):
        if self.path == '/':
            self.path = '/static/index.html'
        elif self.path == '/api/state':
            self.send_response(200)
            self.send_header('Content-type', 'application/json')
            self.end_headers()
            state = {
                "board": board_to_list(),
                "turn": current_turn,
                "gameOver": game_over,
                "message": message,
                "mode": game_mode,
                "trainingMovesLeft": training_total_moves - training_moves_made if game_mode == 'training' else None,
                "moveHistory": move_history
            }
            self.wfile.write(json.dumps(state).encode('utf-8'))
            return
        
        return super().do_GET()

    def do_POST(self):
        global current_turn, game_over, message, training_moves_made, game_mode, move_history
        
        if self.path == '/api/reset':
            content_length = int(self.headers.get('Content-Length', 0))
            if content_length > 0:
                body = self.rfile.read(content_length).decode('utf-8')
                data = json.loads(body)
                mode = data.get('mode', 'local')
            else:
                mode = 'local'
                
            init_game(mode)
            self.send_response(200)
            self.send_header('Content-type', 'application/json')
            self.end_headers()
            self.wfile.write(json.dumps({"status": "ok"}).encode('utf-8'))
            return

        if self.path == '/api/training_reset':
            restore_training_board()
            self.send_response(200)
            self.send_header('Content-type', 'application/json')
            self.end_headers()
            state = {
                "board": board_to_list(),
                "turn": current_turn,
                "gameOver": game_over,
                "message": message,
                "mode": game_mode,
                "trainingMovesLeft": 4 - training_moves_made if game_mode == 'training' else None
            }
            self.wfile.write(json.dumps(state).encode('utf-8'))
            return

        if self.path == '/api/save':
            try:
                stare_arr = (ctypes.c_bool * 6)()
                lib.get_stare_rocada(stare_arr)
                save_data = {
                    "board": board_to_list(),
                    "turn": current_turn,
                    "mode": game_mode,
                    "history": move_history,
                    "castling": [bool(x) for x in stare_arr]
                }
                with open("savegame.json", "w") as f:
                    json.dump(save_data, f)
                self.send_response(200)
                self.send_header('Content-type', 'application/json')
                self.end_headers()
                self.wfile.write(json.dumps({"status": "ok", "message": "Joc salvat cu succes!"}).encode('utf-8'))
            except Exception as e:
                self.send_response(500)
                self.end_headers()
            return
            
        if self.path == '/api/load':
            try:
                import os
                if not os.path.exists("savegame.json"):
                    raise ValueError("Nu exista nicio salvare!")
                with open("savegame.json", "r") as f:
                    save_data = json.load(f)
                    
                game_mode = save_data["mode"]
                current_turn = save_data["turn"]
                move_history = save_data.get("history", [])
                
                # Restauram tabla
                saved_board = save_data["board"]
                for i in range(8):
                    for j in range(8):
                        current_board[i][j] = saved_board[i][j].encode('utf-8')
                        
                # Restauram starea rocadei
                stare_arr = (ctypes.c_bool * 6)(*save_data["castling"])
                lib.set_stare_rocada(stare_arr)
                
                check_game_state()
                
                self.send_response(200)
                self.send_header('Content-type', 'application/json')
                self.end_headers()
                state = {
                    "board": board_to_list(),
                    "turn": current_turn,
                    "gameOver": game_over,
                    "message": "Joc incarcat cu succes!",
                    "mode": game_mode,
                    "trainingMovesLeft": training_total_moves - training_moves_made if game_mode == 'training' else None,
                    "moveHistory": move_history
                }
                self.wfile.write(json.dumps(state).encode('utf-8'))
            except Exception as e:
                self.send_response(500)
                self.send_header('Content-type', 'application/json')
                self.end_headers()
                self.wfile.write(json.dumps({"status": "error", "message": str(e)}).encode('utf-8'))
            return

        if self.path == '/api/auto_move':
            if (game_mode == 'white' and current_turn == 1) or \
               (game_mode == 'black' and current_turn == 0) or \
               (game_mode == 'local' and current_turn == 1):
                res = {"valid": False, "message": "Nu poti folosi sugestia pe tura adversarului!"}
            else:
                if not game_over:
                    if game_mode == 'training':
                        play_training_ai_move()
                    else:
                        play_ai_move()
                        
                    if not game_over and game_mode in ('white', 'black', 'training'):
                        play_ai_move()
                        
                    if game_mode == 'training' and not game_over and training_moves_made >= 4:
                        restore_training_board()
                
                res = {
                    "valid": True,
                    "board": board_to_list(),
                    "turn": current_turn,
                    "gameOver": game_over,
                    "message": message,
                    "mode": game_mode,
                    "trainingMovesLeft": 4 - training_moves_made if game_mode == 'training' else None
                }
            self.send_response(200)
            self.send_header('Content-type', 'application/json')
            self.end_headers()
            self.wfile.write(json.dumps(res).encode('utf-8'))
            return

        if self.path == '/api/move':
            content_length = int(self.headers.get('Content-Length', 0))
            body = self.rfile.read(content_length).decode('utf-8')
            
            try:
                data = json.loads(body)
                from_pos = data['from']
                to_pos = data['to']
                
                if game_over:
                    raise ValueError("Jocul s-a terminat!")
                    
                # Previne mutarea daca nu e randul omului
                if (game_mode == 'white' and current_turn == 1) or (game_mode == 'black' and current_turn == 0):
                    raise ValueError("Asteapta randul tau!")

                if game_mode == 'training' and (to_pos['linie'] > 3 or to_pos['coloana'] > 3):
                    raise ValueError("In antrenament nu poti iesi din zona 4x4!")

                sursa = Pozitie(from_pos['linie'], from_pos['coloana'])
                dest = Pozitie(to_pos['linie'], to_pos['coloana'])
                mutare = Mutare()
                
                valid = lib.mutare_valida(current_board, sursa, dest, current_turn, ctypes.byref(mutare))
                if valid:
                    record_move(current_board, sursa, dest, mutare.piesa_capturata)
                    lib.executa_mutare(current_board, mutare)
                    
                    if game_mode == 'training' and current_turn == 0:
                        training_moves_made += 1
                        
                    current_turn = 1 - current_turn
                    check_game_state()
                    
                    # Daca e vs AI si jocul continua, muta AI-ul imediat
                    if not game_over and ((game_mode in ('white', 'training') and current_turn == 1) or (game_mode == 'black' and current_turn == 0)):
                        play_ai_move()
                        
                    if game_mode == 'training' and not game_over and training_moves_made >= training_total_moves:
                        restore_training_board()
                    
                    res = {
                        "valid": True, 
                        "board": board_to_list(), 
                        "turn": current_turn, 
                        "gameOver": game_over, 
                        "message": message,
                        "mode": game_mode,
                        "trainingMovesLeft": training_total_moves - training_moves_made if game_mode == 'training' else None,
                        "moveHistory": move_history
                    }
                else:
                    res = {"valid": False, "message": "Mutare invalida!"}

            except Exception as e:
                res = {"valid": False, "message": str(e)}

            self.send_response(200)
            self.send_header('Content-type', 'application/json')
            self.end_headers()
            self.wfile.write(json.dumps(res).encode('utf-8'))
            return

        self.send_error(404, "Not Found")

if __name__ == '__main__':
    PORT = 8000
    server = HTTPServer(('127.0.0.1', PORT), ChessHandler)
    print(f"Jocul de sah este pornit! Deschide browserul la adresa: http://127.0.0.1:{PORT}/")
    server.serve_forever()
