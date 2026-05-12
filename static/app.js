const pieceImages = {
    'P': 'https://upload.wikimedia.org/wikipedia/commons/4/45/Chess_plt45.svg',
    'R': 'https://upload.wikimedia.org/wikipedia/commons/7/72/Chess_rlt45.svg',
    'N': 'https://upload.wikimedia.org/wikipedia/commons/7/70/Chess_nlt45.svg',
    'B': 'https://upload.wikimedia.org/wikipedia/commons/b/b1/Chess_blt45.svg',
    'Q': 'https://upload.wikimedia.org/wikipedia/commons/1/15/Chess_qlt45.svg',
    'K': 'https://upload.wikimedia.org/wikipedia/commons/4/42/Chess_klt45.svg',
    'p': 'https://upload.wikimedia.org/wikipedia/commons/c/c7/Chess_pdt45.svg',
    'r': 'https://upload.wikimedia.org/wikipedia/commons/f/ff/Chess_rdt45.svg',
    'n': 'https://upload.wikimedia.org/wikipedia/commons/e/ef/Chess_ndt45.svg',
    'b': 'https://upload.wikimedia.org/wikipedia/commons/9/98/Chess_bdt45.svg',
    'q': 'https://upload.wikimedia.org/wikipedia/commons/4/47/Chess_qdt45.svg',
    'k': 'https://upload.wikimedia.org/wikipedia/commons/f/f0/Chess_kdt45.svg'
};

const boardElement = document.getElementById('chessboard');
const statusText = document.getElementById('game-status');
const turnIndicator = document.getElementById('turn-indicator');
const turnText = document.getElementById('turn-text');
const errorMsg = document.getElementById('error-message');
const turnDot = document.querySelector('.turn-dot');

const setupModal = document.getElementById('setup-modal');

let currentBoardState = [];
let gameMode = 'local'; // 'white', 'black', 'local'
let isDragging = false;

async function fetchState() {
    try {
        const res = await fetch('/api/state');
        const data = await res.json();
        if (data.mode) gameMode = data.mode;
        updateUI(data);
    } catch (e) {
        statusText.innerText = "Eroare la conectarea cu serverul.";
    }
}

async function startNewGame(mode) {
    setupModal.classList.add('hidden');
    gameMode = mode;
    try {
        await fetch('/api/reset', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ mode })
        });
        errorMsg.innerText = "";
        fetchState();
    } catch (e) {
        console.error(e);
    }
}

async function sendMove(from, to) {
    try {
        const res = await fetch('/api/move', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ from, to })
        });
        const data = await res.json();
        
        if (data.valid) {
            errorMsg.innerText = "";
            updateUI(data);
        } else {
            errorMsg.innerText = data.message;
            renderBoard(); // Redeseneaza pt a anula animatia gresita
        }
    } catch (e) {
        errorMsg.innerText = "Eroare retea.";
    }
}

function updateUI(data) {
    currentBoardState = data.board;
    statusText.innerText = data.message || (data.gameOver ? "Joc Terminat" : "Meci in desfasurare");
    
    turnText.innerText = data.turn === 0 ? "Alb" : "Negru";
    turnDot.className = "turn-dot " + (data.turn === 0 ? "alb" : "negru");
    
    if (gameMode === 'training' && data.trainingMovesLeft !== undefined) {
        statusText.innerText = `[${data.trainingMovesLeft} mutări rămase] ` + statusText.innerText;
    }
    
    const btnHint = document.getElementById('btn-hint');
    if (btnHint) {
        let hintEnabled = false;
        if (gameMode === 'white' && data.turn === 0) hintEnabled = true;
        if (gameMode === 'black' && data.turn === 1) hintEnabled = true;
        if (gameMode === 'local' && data.turn === 0) hintEnabled = true; // In modul local, doar Albul are hint conform cerintei
        if (gameMode === 'training' && data.turn === 0) hintEnabled = true;

        if (hintEnabled) {
            btnHint.disabled = false;
            btnHint.style.opacity = '1';
            btnHint.style.cursor = 'pointer';
        } else {
            btnHint.disabled = true;
            btnHint.style.opacity = '0.5';
            btnHint.style.cursor = 'not-allowed';
        }
    }
    
    const btnNext = document.getElementById('btn-next-training');
    const btnRestart = document.getElementById('btn-restart-training');
    if (btnNext && btnRestart) {
        if (gameMode === 'training') {
            btnRestart.style.display = 'inline-block';
            if (data.gameOver) {
                btnNext.style.display = 'inline-block';
            } else {
                btnNext.style.display = 'none';
            }
        } else {
            btnRestart.style.display = 'none';
            btnNext.style.display = 'none';
        }
    }
    
    // Update Move History
    const historyList = document.getElementById('move-history-list');
    if (historyList && data.moveHistory) {
        historyList.innerHTML = '';
        for (let i = 0; i < data.moveHistory.length; i += 2) {
            const li = document.createElement('li');
            li.style.padding = '4px 0';
            li.style.borderBottom = '1px solid #f8f9fa';
            
            const moveNum = Math.floor(i/2) + 1;
            const whiteMove = data.moveHistory[i];
            const blackMove = data.moveHistory[i+1] ? data.moveHistory[i+1] : '';
            
            li.innerHTML = `<span style="color: #95a5a6; display: inline-block; width: 30px; font-size: 13px;">${moveNum}.</span> 
                            <span style="font-weight: 600; color: #2980b9; display: inline-block; width: 70px;">${whiteMove}</span> 
                            <span style="font-weight: 600; color: #e74c3c;">${blackMove}</span>`;
            historyList.appendChild(li);
        }
        // Auto scroll
        historyList.parentElement.scrollTop = historyList.parentElement.scrollHeight;
    }
    
    renderBoard();
}

function renderBoard() {
    boardElement.innerHTML = '';
    
    const maxDim = (gameMode === 'training') ? 4 : 8;
    boardElement.style.gridTemplateColumns = `repeat(${maxDim}, 1fr)`;
    boardElement.style.gridTemplateRows = `repeat(${maxDim}, 1fr)`;
    
    // Daca jucam cu Negrul, rotim tabla
    const isFlipped = (gameMode === 'black');
    
    for (let row = 0; row < maxDim; row++) {
        for (let col = 0; col < maxDim; col++) {
            const square = document.createElement('div');
            
            // Coordonatele reale de pe tabla (backend)
            const realRow = isFlipped ? (maxDim - 1) - row : row;
            const realCol = isFlipped ? (maxDim - 1) - col : col;
            
            const isLight = (realRow + realCol) % 2 === 0;
            square.className = `square ${isLight ? 'light' : 'dark'}`;
            
            square.dataset.linie = realRow;
            square.dataset.coloana = realCol;
            
            // Drag and drop events pe patrat (destinatie)
            square.addEventListener('dragover', (e) => {
                e.preventDefault(); // Permite drop
                square.classList.add('drag-over');
            });
            square.addEventListener('dragleave', () => {
                square.classList.remove('drag-over');
            });
            square.addEventListener('drop', (e) => {
                e.preventDefault();
                square.classList.remove('drag-over');
                isDragging = false;
                
                const fromLinie = parseInt(e.dataTransfer.getData('linie'));
                const fromColoana = parseInt(e.dataTransfer.getData('coloana'));
                
                if (fromLinie !== realRow || fromColoana !== realCol) {
                    sendMove({ linie: fromLinie, coloana: fromColoana }, { linie: realRow, coloana: realCol });
                }
            });
            
            const piesa = currentBoardState[realRow][realCol];
            if (piesa !== ' ') {
                const img = document.createElement('img');
                img.src = pieceImages[piesa];
                img.className = 'piece';
                img.draggable = true;
                
                // Drag start pe piesa
                img.addEventListener('dragstart', (e) => {
                    isDragging = true;
                    e.dataTransfer.setData('linie', realRow);
                    e.dataTransfer.setData('coloana', realCol);
                    // Efect vizual usor transparent la tragere
                    setTimeout(() => { img.style.opacity = '0.5'; }, 0);
                });
                img.addEventListener('dragend', () => {
                    img.style.opacity = '1';
                    isDragging = false;
                });
                
                square.appendChild(img);
            }
            
            boardElement.appendChild(square);
        }
    }
}

// Butoane Modal
document.getElementById('btn-play-white').addEventListener('click', () => startNewGame('white'));
document.getElementById('btn-play-black').addEventListener('click', () => startNewGame('black'));
document.getElementById('btn-play-local').addEventListener('click', () => startNewGame('local'));
document.getElementById('btn-play-training').addEventListener('click', () => startNewGame('training'));

document.getElementById('btn-reset').addEventListener('click', () => {
    setupModal.classList.remove('hidden');
});

document.getElementById('btn-next-training').addEventListener('click', () => {
    startNewGame('training');
});

document.getElementById('btn-restart-training').addEventListener('click', async () => {
    try {
        const res = await fetch('/api/training_reset', { method: 'POST' });
        const data = await res.json();
        updateUI(data);
    } catch (e) {
        showError("Eroare la resetarea puzzle-ului.");
    }
});

document.getElementById('btn-hint').addEventListener('click', async () => {
    try {
        const res = await fetch('/api/auto_move', { method: 'POST' });
        const data = await res.json();
        if (data.valid) {
            updateUI(data);
        } else {
            errorMsg.innerText = data.message || "Nu s-a putut genera o mutare.";
        }
    } catch (e) {
        errorMsg.innerText = "Eroare retea.";
    }
});

document.getElementById('btn-save').addEventListener('click', async () => {
    try {
        const res = await fetch('/api/save', { method: 'POST' });
        const data = await res.json();
        if (data.status === 'ok') {
            alert(data.message);
        } else {
            errorMsg.innerText = "Eroare la salvare: " + data.message;
        }
    } catch (e) {
        errorMsg.innerText = "Eroare retea.";
    }
});

document.getElementById('btn-load').addEventListener('click', async () => {
    try {
        const res = await fetch('/api/load', { method: 'POST' });
        const data = await res.json();
        if (data.status === 'ok' || data.board) {
            updateUI(data);
            alert(data.message || "Joc incarcat.");
        } else {
            errorMsg.innerText = "Eroare la incarcare: " + data.message;
        }
    } catch (e) {
        errorMsg.innerText = "Eroare retea.";
    }
});

// Init
fetchState();
