'use strict';
const cardWidth = 150;
const cardHeight = cardWidth * 730 / 500;
const bigCardWidth = 250;
const bigCardHeight = bigCardWidth * cardHeight / cardWidth;
let url = 'http://localhost:8080';

let session = '';
let gameID = '';
let user = '';
let confirmFunction = () => { };
let lastUpdated = 0;
let blinkHandle = null;
const prefix = "[!]";
let gameState = null;
let gameTimerID = null;

const actionNames = {
    "draw": "Взять карту",
    "discard": "Положить в шкаф",
    "assemble": "Собрать рецепт",
    "disassemble": "Разобрать рецепт",
    "skip": "Пропустить ход",
    "cast": "Прочесть заклинание",
    "endturn": "Завершить ход",
};

const stateNames = {
    "drawing": "взятия карты",
    "drawplaying": "взятия или хода",
    "playing": "хода",
    "done": "конца хода"
};

const specialStateNames = {
    "drawextra": "Взятия карт",
    "disassembling": "Разбора рецептов",
    "giving": "Отдачи ингридиента",
    "transfiguring": "Трансфигурации"
};

const turn = {};

function randStr() {
    return Math.random().toString(36).substring(2, 15) + Math.random().toString(36).substring(2, 15);
}

function usePartInsteadOfCard() {
    return turn.card == 75 || turn.card == 76;
}

function addBubble(str) {
    function removeBubble(id) {
        const bubble = document.getElementById(id);
        bubble.remove();
    }
    const id = randStr();
    const bubble = document.createElement("div");
    bubble.classList.add("bubble");
    bubble.id = id;
    const closeBtn = document.createElement("a");
    closeBtn.href = "#";
    closeBtn.text = 'X';
    closeBtn.classList.add("X");
    closeBtn.addEventListener('click', (e) => {
        e.preventDefault();
        removeBubble(id);
    });
    bubble.appendChild(closeBtn);
    bubble.appendChild(document.createTextNode(' Request failed: ' + str));
    document.getElementById("bubbles").appendChild(bubble);
    setTimeout(function () {
        removeBubble(id);
    }, 5000);
}

function removeNotification() {
    const notification = document.getElementById("notif_div");
    if (notification) {
        notification.remove();
        clearInterval(blinkHandle);
        document.title = document.title.replace(prefix, '');
    }
}

function addNotification(str) {
    removeNotification();
    const notification = document.createElement("div");
    notification.classList.add("notification");
    notification.id = "notif_div";
    notification.addEventListener('click', (e) => {
        removeNotification();
    });
    notification.appendChild(document.createTextNode(str));
    document.body.appendChild(notification);
}

function resetTurn(state) {
    turn.card = -1;
    if (state && state["state"] == "done") {
        turn.action = "endturn";
    } else {
        turn.action = "draw";
        turn.deck = "base";
    }
    turn.parts = [];
    updateTurnPlanner();
}

function createImg(id, width, height) {
    const img = document.createElement("img");
    img.src = "res/" + id + ".png";
    img.width = width;
    img.height = height;
    return img;
}

function createCard(id, width) {
    if (!width) {
        width = cardWidth;
    }
    let height = width * cardHeight / cardWidth;
    const adiv = document.createElement("div");
    adiv.classList.add('card');
    const img1 = createImg('c' + id, width, height);
    adiv.appendChild(img1);
    return adiv;
}

function createSubCard(id) {
    const adiv = document.createElement("div");
    adiv.classList.add('subCard');
    const img1 = createImg('c' + id, cardWidth * 0.3, cardHeight * 0.3);
    adiv.appendChild(img1);
    return adiv;
}

function createHoverDiv(trigger, noUp) {
    const hoverDiv = document.createElement("div");
    hoverDiv.classList.add("hover");
    trigger.addEventListener('mouseover', (e) => {
        hoverDiv.style.display = 'block';
    });
    trigger.addEventListener('mouseout', (e) => {
        hoverDiv.style.display = 'none';
    });
    trigger.addEventListener('mousemove', (e) => {
        if (e.pageX > bigCardWidth * 2) {
            hoverDiv.style.left = e.pageX - bigCardWidth - 20;
        } else {
            hoverDiv.style.left = e.pageX + 20;
        }
        if (e.pageY > bigCardHeight * 2 && !noUp) {
            hoverDiv.style.top = e.pageY - bigCardHeight - 20;
        } else {
            hoverDiv.style.top = e.pageY + 20;
        }
    });
    document.body.appendChild(hoverDiv);
    return hoverDiv;
}

function createClosetCard(cardId, partsIds) {
    const card = createCard(cardId);
    const hoverDiv = createHoverDiv(card, true);
    for (let i in partsIds) {
        const partId = partsIds[i];
        const part = createCard(partId, bigCardWidth);
        hoverDiv.appendChild(part);
    }
    return card;
}

function createAssembled(cardId, partsIds) {
    const card = createCard(cardId);
    const hoverDiv = createHoverDiv(card);
    hoverDiv.appendChild(createCard(cardId, bigCardWidth));
    for (let i in partsIds) {
        let backName = "base";
        if (partsIds[i] > 76) {
            backName = "university";
        }
        const subDiv = createSubCard(backName);
        card.appendChild(subDiv);
        subDiv.style.top = cardHeight * 0.7 + "px";
        const hoverSubDiv = createHoverDiv(subDiv);
        const partId = partsIds[i];
        const part = createCard(partId, bigCardWidth);
        hoverSubDiv.appendChild(part);
        subDiv.addEventListener('click', () => {
            if (usePartInsteadOfCard()) {
                addPart(+partsIds[i], "recipe");
            }
        });
    }
    return card;
}

function recreateTable() {
    [].forEach.call(document.querySelectorAll('.hover'), function (e) {
        e.parentNode.removeChild(e);
    });
    const table = document.getElementById("table");
    table.innerHTML = "";
    const players = document.createElement("div");
    players.id = "players";
    players.classList.add("players");
    table.appendChild(players);
    table.appendChild(document.createElement("br"));
    const closet = document.createElement("div");
    closet.id = "closet";
    closet.classList.add("section");
    const closetTitle = document.createElement("h3");
    closetTitle.innerText = "Шкаф";
    closet.appendChild(closetTitle);
    table.appendChild(closet);
    table.appendChild(document.createElement("br"));
    const me = document.createElement("div");
    me.id = "me";
    me.classList.add('section');
    table.appendChild(me);
    table.appendChild(document.createElement("br"));
}

function addPart(id, type) {
    if (turn.card == -1) {
        return;
    }
    if (turn.parts.find(e => {
        return e.id == id
    })) {
        return;
    }
    turn.parts.push({
        id: id,
        type: type
    });
    if (data.cards[turn.card].type == "spell") {
        turn.action = "cast";
    } else {
        turn.action = "assemble";
    }
    updateTurnPlanner();
}

function drawCloset(closet) {
    const closetDiv = document.getElementById("closet");
    for (let i in closet) {
        const cards = closet[i];
        const topIdx = cards[cards.length - 1];
        const card = createClosetCard(topIdx, cards);
        card.setAttribute("name", "i_" + i);
        card.addEventListener('click', (e) => {
            addPart(topIdx, "ingredient");
        });
        closetDiv.appendChild(card);
    }
}

function drawTable(tableDiv, cards) {
    for (let i in cards) {
        const parts = cards[i];
        const assembledDiv = createAssembled(i, parts);
        assembledDiv.setAttribute("name", "c_" + i);
        assembledDiv.addEventListener('click', (e) => {
            if (gameState["specialstate"]["state"] == "disassembling") {
                turn.card = +i;
                turn.action = "disassemble";
                updateTurnPlanner();
            } else if (!usePartInsteadOfCard()) {
                addPart(+i, "recipe");
            }
        });
        tableDiv.appendChild(assembledDiv);
    }
}

function drawMyTable(me) {
    const myHand = me["hand"];
    const myDiv = document.getElementById("me");
    const myAssembledDiv = document.createElement("div");
    const myHandDiv = document.createElement("div");
    const assembledTitle = document.createElement("h3");
    assembledTitle.innerText = "Собрано";
    myAssembledDiv.appendChild(assembledTitle);
    const handTitle = document.createElement("h3");
    handTitle.innerText = "В руке";
    myHandDiv.appendChild(handTitle);
    myDiv.appendChild(myAssembledDiv);
    myDiv.appendChild(document.createElement("br"));
    myDiv.appendChild(myHandDiv);
    for (let i in myHand) {
        const cardInHandIdx = myHand[i];
        const cardinHand = createCard(cardInHandIdx);
        const hoverDiv = createHoverDiv(cardinHand);
        hoverDiv.appendChild(createCard(cardInHandIdx, bigCardWidth));
        myHandDiv.appendChild(cardinHand);
        cardinHand.addEventListener('click', (e) => {
            if (turn.action == "cast" && turn.card >= 86 && turn.card <= 88) {
                addPart(+cardInHandIdx, "recipe");
            } else {
                removeHighLight();
                highlightRequired(cardInHandIdx);
                turn.card = cardInHandIdx;
                turn.action = "discard";
                updateTurnPlanner();
            }
        })
    }

    const myAssembled = me["table"];
    drawTable(myAssembledDiv, myAssembled);
}

function drawOpponent(player) {
    const opponentsDiv = document.getElementById("players");
    const opponentDiv = document.createElement("div");
    opponentDiv.classList.add("section");
    const userTitle = document.createElement("h3");
    userTitle.innerText = player["user"];
    opponentDiv.appendChild(userTitle);
    opponentsDiv.appendChild(opponentDiv);
    const handDiv = document.createElement("div");
    opponentDiv.appendChild(handDiv);
    opponentDiv.appendChild(document.createElement("br"));
    const assembledDiv = document.createElement("div");
    opponentDiv.appendChild(assembledDiv);
    const assembledTitle = document.createElement("h3");
    assembledTitle.innerText = "Собрано";
    assembledDiv.appendChild(assembledTitle);
    for (let typeIdx in player["hand"]) {
        for (let i = 0; i < player["hand"][typeIdx]; ++i) {
            const cardDiv = createCard(typeIdx);
            cardDiv.style.width = 40;
            handDiv.appendChild(cardDiv);
        }
    }
    drawTable(assembledDiv, player["table"]);
}

function drawPlayers(players) {
    for (let i in players) {
        const player = players[i];
        if (player.user == user) {
            drawMyTable(player);
        } else {
            drawOpponent(player);
        }
    }
}

function drawBoard(state) {
    recreateTable();
    drawCloset(state["closet"]);
    let infoHtml = "";
    for (let i in state["players"]) {
        infoHtml += state["players"][i].user + " " + state["players"][i].score + "<br />";
    }
    document.getElementById("score_info").innerHTML = infoHtml;
    const deckDiv = document.getElementById("deck_info");
    deckDiv.innerHTML = "";
    for (let i in state["decks"]) {
        const deck = state["decks"][i];
        const deckImg = document.createElement("img");
        deckImg.src = "res/c" + i + ".png";
        deckImg.width = 40;
        deckImg.height = deckImg.width * cardHeight / cardWidth;
        deckImg.addEventListener('click', (e) => {
            turn.action = "draw";
            turn.deck = i;
            turn.parts = [];
            updateTurnPlanner();
        });
        deckDiv.appendChild(deckImg);
        deckDiv.appendChild(document.createTextNode(deck));
    }
    drawPlayers(state["players"]);
    let turnTxt = "";
    const goButton = document.getElementById("make_turn_btn");

    if (state["specialstate"]["state"] != "none") {
        const specialUser = state["players"][state["specialstate"]["player"]]["user"];
        turnTxt = "Особая фаза " + specialStateNames[state["specialstate"]["state"]] + ". Ход " + specialUser;
        if (specialUser == user) {
            goButton.disabled = false;
        } else {
            goButton.disabled = true;
        }
    } else {
        if (state["turn"] == user) {
            turnTxt = "Мой ход!";
            goButton.disabled = false;
        } else {
            turnTxt = "Ход " + state["turn"];
            goButton.disabled = true;
        }
        turnTxt += '<br/>Фаза ' + stateNames[state["state"]];
    }
    document.getElementById("turn_header").innerHTML = turnTxt;
}

function highlightRequired(id) {
    const reqs = data.cards[id].requirements;
    for (let i in reqs) {
        const req = reqs[i];
        if (req.type == "ingredient") {
            if (!(16 in req.ids)) {
                req.ids.push(16);
            }
            for (let j in req.ids) {
                const elms = document.getElementsByName("i_" + req.ids[j]);
                [].forEach.call(elms, (elm) => {
                    elm.classList.add('highlighted');
                });
            }
        } else if (req.type == "recipe") {
            for (let j in req.ids) {
                const elms = document.getElementsByName("c_" + req.ids[j]);
                [].forEach.call(elms, (elm) => {
                    elm.classList.add('highlighted');
                });
            }
        }
    }
}

function removeHighLight() {
    [].forEach.call(document.querySelectorAll(".highlighted"), element => {
        element.classList.remove("highlighted");
    });
}

function updateTurnPlanner() {
    const turnCardContainer = document.getElementById("turnCard");
    turnCardContainer.innerHTML = "";
    if (turn.action == "cast" && turn.card >= 80 && turn.card <= 82) {
        document.getElementById("ingredientSelect").classList.remove('hidden');
    } else {
        document.getElementById("ingredientSelect").classList.add('hidden');
    }
    if (turn.action == "assemble" || turn.action == "discard" || turn.action == "cast" || turn.action == "disassemble") {
        const turnCardDiv = createCard(turn.card);
        turnCardContainer.appendChild(turnCardDiv);
    } else if (turn.action == "draw") {
        const turnCardDiv = createCard(turn.deck);
        turnCardContainer.appendChild(turnCardDiv);
    }
    const actionSelect = document.getElementById("actionSelect");
    const opts = actionSelect.options;
    for (let opt, j = 0; opt = opts[j]; j++) {
        if (opt.value == turn.action) {
            actionSelect.selectedIndex = j;
            break;
        }
    }

    const turnPartsDiv = document.getElementById("turnParts");
    turnPartsDiv.innerHTML = "";
    for (let i in turn.parts) {
        const part = createCard(turn.parts[i].id, cardWidth / 2);
        part.addEventListener('click', (e) => {
            turn.parts = turn.parts.filter((e) => {
                return e.id != turn.parts[i].id;
            });
            if (turn.parts.length == 0) {
                turn.action = "discard";
            }
            updateTurnPlanner();
        });
        turnPartsDiv.appendChild(part);
    }
}

function testDraw(state) {
    /*const state = {
        "closet": {
            "5": [10, 44],
            "8": [16],
            "9": [18, 46, 60]
        },
        "deck": 25,
        "opponents": [{
            "name": "Вася",
            "hand": 6,
            "table": {
                "8": [6, 63],
                "34": [48, 26, 45],
                "27": [22, 8]
            }
        }],
        "me": {
            "hand": [32, 53, 1, 7, 15],
            "table": {
                "17": [67, 30],
                "31": [24, 20]
            }
        }
    }*/
    drawBoard(state);
    resetTurn();
}

function request(url, options, f) {
    fetch(url, options).then((response) => {
        if (response.status != 200) {
            f = addBubble;
            if (response.status == 401) {
                showLogin();
            }
        }
        return response.text();
    }).then((body) => {
        f(body);
    }).catch(error => {
        addBubble(error);
        showLogin();
    });
}

function login() {
    user = document.getElementById("login_txt").value;
    request(url + '/login', {
        method: "Post",
        body: JSON.stringify({
            user: user
        })
    }, (body) => {
        console.log(body);
        const res = JSON.parse(body);
        session = res['session_id'];
        if (session) {
            showGames();
        }
    });
}

function blink() {
    const title = document.title;
    if (title.startsWith('[')) {
        document.title = document.title.replace(prefix, '');
    } else {
        document.title = prefix + document.title;
    }
}

function setGameTimer() {
    if (gameTimerID) {
        clearTimeout(gameTimerID);
    }
    gameTimerID = setTimeout(gameTimer, 5000);
}

function gameTimer() {
    gameTimerID = null;
    request(url + '/game/lastupdate', {
        method: "Post",
        body: JSON.stringify({
            session: session,
            game_id: gameID
        })
    }, (body) => {
        const res = JSON.parse(body);
        const newLastUpdate = res["updated"];
        if (newLastUpdate > lastUpdated) {
            redrawBoard();
            addNotification("New activity");
            blinkHandle = setInterval(blink, 1000);
        } else {
            setGameTimer();
        }
    });
}

function recreateTurnHistory(turns) {
    const historyDiv = document.getElementById("turn_history");
    historyDiv.innerHTML = "";
    let addText = (t) => {
        historyDiv.appendChild(document.createTextNode(t));
    }
    let addCard = (id) => {
        const w = 40;
        const h = w * cardHeight / cardWidth;
        const card = createImg('c' + id, w, h);
        historyDiv.appendChild(card);
        const hover = createHoverDiv(card);
        hover.appendChild(createCard(id, bigCardWidth));
    }
    let addIngredient = (id) => {
        const w = 40;
        const h = w * cardHeight / cardWidth;
        const card = createImg('i' + id, w, h);
        historyDiv.appendChild(card);
    }
    for (let i = turns.length - 1; i >= Math.max(0, turns.length - 20); --i) {
        const turn = turns[i];
        addText(turn["user"]);
        if (turn["action"] == "draw") {
            addText(" взял карту");
            addCard(turn["deck"]);
        } else if (turn["action"] == "discard") {
            addText(" сбросил ");
            addCard(turn["card"]);
        } else if (turn["action"] == "disasemble") {
            addText(" разобрал ");
            addCard(turn["card"]);
        } else if (turn["action"] == "assemble") {
            addText(" собрал ");
            addCard(turn["card"]);
            addText(" из ");
            for (let j in turn["parts"]) {
                const part = turn["parts"][j];
                addCard(part.id);
            }
        } else if (turn["action"] == "cast") {
            addText(" прочел ");
            addCard(turn["card"]);
            if (turn["card"] >= 80 && turn["card"] <= 82) {
                addIngredient(turn["ingredient"]);
            } else {
                addText(" на ");
                for (let j in turn["parts"]) {
                    const part = turn["parts"][j];
                    addCard(part.id);
                }
            }
        } else if (turn["action"] == "skip") {
            addText(" пропустил фазу");
        } else if (turn["action"] == "endturn") {
            addText(" завершил ход");
        }
        historyDiv.appendChild(document.createElement("br"));
    }
}

function redrawBoard() {
    request(url + '/game/query', {
        method: "Post",
        body: JSON.stringify({
            session: session,
            game_id: gameID
        })
    }, (body) => {
        const res = JSON.parse(body);
        const state = res["game"];
        gameState = state;
        lastUpdated = state["updated"];
        console.log(body);
        drawBoard(state);
        recreateTurnHistory(res["moves"]);
        resetTurn(state);
        setGameTimer();
    });
}

function makeTurn() {
    request(url + '/game/turn', {
        method: "Post",
        body: JSON.stringify({
            session: session,
            game_id: gameID,
            turn: turn
        })
    }, (body) => {
        console.log(body);
        redrawBoard();
    });
}

function undo() {
    request(url + '/game/undo', {
        method: "Post",
        body: JSON.stringify({
            session: session,
            game_id: gameID
        })
    }, (body) => {
        console.log(body);
        redrawBoard();
    });
}

function removeGame(id) {
    request(url + '/game/delete', {
        method: "Post",
        body: JSON.stringify({
            session: session,
            game_id: id
        })
    }, (body) => {
        console.log(body);
        showGames();
    });
}

function createGame() {
    const body = {
        session: session,
        name: document.getElementById("new_game_name").value
    };
    if (document.getElementById("university_cb").checked) {
        body.university = true;
    }
    request(url + '/game/create', {
        method: "Post",
        body: JSON.stringify(body)
    }, (body) => {
        console.log(body);
        showGames();
    });
}

function startGame(id) {
    request(url + '/game/start', {
        method: "Post",
        body: JSON.stringify({
            session: session,
            game_id: id
        })
    }, (body) => {
        console.log(body);
        showGames();
    });
}

function joinGame(id) {
    request(url + '/game/join', {
        method: "Post",
        body: JSON.stringify({
            session: session,
            game_id: id
        })
    }, (body) => {
        console.log(body);
        showGames();
    });
}

function launchGame(id) {
    gameID = id;
    hideAllStates();
    document.getElementById("game_div").classList.remove('hidden');
    redrawBoard();
}

function showGamesToJoin() {
    hideAllStates();
    const gamesDiv = document.getElementById("games_div");
    gamesDiv.classList.remove('hidden');
    const gamesListDiv = document.getElementById("games_list_div");
    gamesListDiv.innerHTML = "";
    request(url + '/game/list', {
        method: "Post",
        body: JSON.stringify({
            session: session,
            showPreparing: true
        })
    }, (body) => {
        console.log(body);
        const games = JSON.parse(body)["games"];
        for (let i = 0; i < games.length; ++i) {
            const game = games[i];
            const gameLink = document.createElement("a");
            gameLink.href = '#';
            gameLink.text = game.name;
            gameLink.addEventListener('click', (e) => {
                e.preventDefault();
                joinGame(game.id);
            })
            gamesListDiv.appendChild(gameLink);
            gamesListDiv.appendChild(document.createElement("br"));
        }
    });
}

function confirmation() {
    const txt = document.getElementById('confirm_txt');
    if (txt.value != 'Gregory is the boss') {
        addBubble('Incorrect');
        return;
    }
    txt.value = '';
    document.getElementById("confirm").classList.add("hidden");
    confirmFunction();
    confirmFunction = () => { };
}

function showGames() {
    hideAllStates();
    const gamesDiv = document.getElementById("games_div");
    gamesDiv.classList.remove('hidden');
    const gamesListDiv = document.getElementById("games_list_div");
    gamesListDiv.innerHTML = "";

    request(url + '/game/list', {
        method: "Post",
        body: JSON.stringify({
            session: session
        })
    }, (body) => {
        console.log(body);
        const games = JSON.parse(body)["games"];
        for (let i = 0; i < games.length; ++i) {
            const game = games[i];
            const gameDiv = document.createElement("div");
            gameDiv.classList.add("box");
            gamesListDiv.append(gameDiv);
            const gameHeader = document.createElement("h3");
            gameHeader.innerHTML = game.name;
            gameDiv.appendChild(gameHeader);
            gameDiv.appendChild(document.createTextNode("State: " + game.state));
            for (let j = 0; j < game.players.length; ++j) {
                gameDiv.appendChild(document.createElement("br"));
                gameDiv.appendChild(document.createTextNode(game.players[j]));
            }
            gameDiv.appendChild(document.createElement("br"));
            const removeBtn = document.createElement("input");
            removeBtn.type = "button";
            removeBtn.value = "Remove";
            removeBtn.addEventListener('click', (e) => {
                const confirmDiv = document.getElementById("confirm");
                confirmDiv.classList.remove("hidden");
                confirmDiv.style.top = document.body.scrollHeight / 2;
                confirmFunction = () => {
                    removeGame(game.id);
                }
            });
            gameDiv.appendChild(removeBtn);
            if (game.state == "preparing" && game.players.length > 1) {
                const startBtn = document.createElement("input");
                startBtn.type = "button";
                startBtn.value = "Start";
                startBtn.addEventListener('click', (e) => {
                    startGame(game.id);
                });
                gameDiv.appendChild(startBtn);
            }
            if (game.state != "preparing") {
                const launchBtn = document.createElement("input");
                launchBtn.type = "button";
                launchBtn.value = "Launch";
                launchBtn.addEventListener('click', (e) => {
                    launchGame(game.id);
                });
                gameDiv.appendChild(launchBtn);
            }
            gamesListDiv.appendChild(document.createElement("br"));
        }
    });
}

function showLogin() {
    hideAllStates();
    document.getElementById("login_div").classList.remove('hidden');
}

function hideAllStates() {
    document.getElementById("login_div").classList.add('hidden');
    document.getElementById("game_div").classList.add('hidden');
    document.getElementById("games_div").classList.add('hidden');
}

document.addEventListener("DOMContentLoaded", () => {
    const actionSelect = document.getElementById("actionSelect");
    const ingredientSelect = document.getElementById("ingredientSelect");
    for (let i in actionNames) {
        const option = document.createElement("option");
        option.value = i;
        option.innerText = actionNames[i];
        actionSelect.appendChild(option);
    }
    actionSelect.addEventListener('change', () => {
        turn.action = actionSelect.value;
        updateTurnPlanner();
    });

    for (let i in data.ingredients) {
        const option = document.createElement("option");
        option.value = i;
        option.innerText = data.ingredients[i];
        ingredientSelect.appendChild(option);
    }
    ingredientSelect.addEventListener('change', () => {
        turn.ingredient = +ingredientSelect.value;
    });

    document.getElementById("resetTurn").addEventListener('click', (e) => {
        resetTurn();
        removeHighLight();
        updateTurnPlanner();
    });

    document.getElementById("login_btn").addEventListener('click', (e) => {
        url = 'https://shtras.net:8080';
        login();
    });
    document.getElementById("debug_login_btn").addEventListener('click', (e) => {
        url = 'http://localhost:8080';
        login();
    });
    document.getElementById("refresh_games_btn").addEventListener('click', (e) => {
        showGames();
    });
    document.getElementById("new_game_btn").addEventListener('click', (e) => {
        createGame();
    });
    document.getElementById("joinable_games_btn").addEventListener('click', (e) => {
        showGamesToJoin();
    });
    document.getElementById("game_refresh_btn").addEventListener('click', (e) => {
        redrawBoard();
    });
    document.getElementById("make_turn_btn").addEventListener('click', (e) => {
        makeTurn();
    });
    document.getElementById("undo_btn").addEventListener('click', (e) => {
        undo();
    });
    document.getElementById("confirm_btn").addEventListener('click', () => {
        confirmation();
    });
    document.getElementById("cancel_confirm_btn").addEventListener('click', () => {
        document.getElementById("confirm").classList.add("hidden");
    });
    document.onclick = function () {
        removeNotification();
    };
    document.getElementById("collapse_turn").addEventListener('click', (e) => {
        e.preventDefault();
        const turnInner = document.getElementById('turn_inner');
        if (turnInner.classList.contains('hidden')) {
            turnInner.classList.remove('hidden');
        } else {
            turnInner.classList.add('hidden');
        }
    });
    document.getElementById("login_div").classList.remove('hidden');
})