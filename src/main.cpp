/*****************************************************************//**
 * \file   main.cpp
 * \brief  Chess game implementation
 * 
 * \author bytenol
 * \date   April 2025, 08
 * 
 * character sprites are gotten from https://opengameart.org/content/chess-pieces-and-board-squares
 * I designed some myself though e.g vision, selected
 * 
 * TODO
 *  - visual enhancement
 *      * Flip bishop and maybe knights and others
 *********************************************************************/

#include "main.hpp"

constexpr unsigned int TILESIZE = 64;
constexpr unsigned int ROW = 8;
constexpr unsigned int COL = 8;

Player *currentPlayer, *nextPlayer, *whitePlayer, *blackPlayer;
Player player1, player2;


std::vector<std::vector<int>> CollisionBoard::colorBuffer;
std::vector<std::vector<CharacterName>> CollisionBoard::nameBuffer;

struct {

    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    bool windowShouldClose = false;
    SDL_Event evt;

} canvas;



Character* currentChr = nullptr;
std::vector<std::unique_ptr<Character>> characters;
std::map<std::string, SDL_Texture*> textures;



int main(int argc, char* argv[])
{
    if (!init()) return -1;
    loadTextures();
    initPlayers();
    mainLoop();

    return 0;
}


Character::Character(Point2D p, CharacterName _name, bool _isWhite, bool _isTop)
{
    name = _name;
    isWhite = _isWhite;
    isTop = _isTop;

    pos = p;
    startPos.x = p.x;
    startPos.y = p.y;
}

void Character::Draw(SDL_Renderer* renderer)
{
    SDL_Texture* texture = nullptr;
    switch (name)
    {
    case CharacterName::PAWN:
        texture = textures[isWhite ? "w_pawn" : "b_pawn"];
        break;
    case CharacterName::ROOK:
        texture = textures[isWhite ? "w_rook" : "b_rook"];
        break;
    case CharacterName::KNIGHT:
        texture = textures[isWhite ? "w_knight" : "b_knight"];
        break;
    case CharacterName::BISHOP:
        texture = textures[isWhite ? "w_bishop" : "b_bishop"];
        break;
    case CharacterName::KING:
        texture = textures[isWhite ? "w_king" : "b_king"];
        break;
    case CharacterName::QUEEN:
        texture = textures[isWhite ? "w_queen" : "b_queen"];
        break;
    }

    SDL_Rect dstRect{ pos.x * TILESIZE, pos.y * TILESIZE, TILESIZE, TILESIZE };
    SDL_RenderCopy(renderer, texture, nullptr, &dstRect);
}


void Character::SetPos(int x, int y)
{
    pos.x = x;
    pos.y = y;
}


decltype(Character::pos)& Character::GetPos()
{
    return pos;
}

bool Character::IsSame(const Character& other)
{
    if (isWhite && other.isWhite) return true;
    if (!isWhite && !(other.isWhite)) return true;
    return false;
}


Character::path_t Character::GetRookPath(Character& character)
{
    path_t v;

    // up and down
    auto& pos = character.GetPos();

    Point2D p{ pos.x, pos.y };

    auto loopThroughAxis = [&](const std::string& a = "y") {
        for (int i = 0; i < 2; i++)
        {
            p.x = pos.x;
            p.y = pos.y;

            bool isHit = false;
            int dir = i == 0 ? 1 : -1;
            auto axis = a == "y" ? &p.y : &p.x;
            auto mx_axis = (a == "y") ? CollisionBoard::ROW_SIZE : CollisionBoard::COL_SIZE;
            *axis += dir;
            
            while (!isHit && *axis >= 0 && *axis < mx_axis)
            {
                auto color = CollisionBoard::GetColorAt(p.x, p.y);
                if (color < 0)
                    v.push_back(p);
                else
                {
                    if (!(character.GetColor() == color)) v.push_back(p);
                    isHit = true;
                }
                *axis += dir;
            }
        }

    };

    loopThroughAxis("y");
    loopThroughAxis("x");

    return v;
}


Character::path_t Character::GetBishopPath(Character& character)
{
    path_t v;

    auto& pos = character.GetPos();
    Point2D p{ pos.x, pos.y };

    auto loopThroughAxis = [&](const std::string& a = "top") {
        for (int i = 0; i < 2; i++)
        {
            p.x = pos.x;
            p.y = pos.y;

            int dir = i == 0 ? -1 : 1;
            bool isHit = false;
            p.x += dir;
            p.y += (a == "top")? -1: 1;

            while (!isHit && p.x >= 0 && p.x < CollisionBoard::COL_SIZE && p.y >= 0 && p.y < CollisionBoard::ROW_SIZE)
            {
                auto color = CollisionBoard::GetColorAt(p.x, p.y);
                if (color < 0)
                    v.push_back(p);
                else
                {
                    if (!(character.GetColor() == color)) v.push_back(p);
                    isHit = true;
                }
                p.x += dir;
                p.y += (a == "top") ? -1 : 1;
            }

        }
    };

    loopThroughAxis("top");
    //loopThroughAxis("down");

    return v;
}

bool Character::MoveTo(Point2D dest)
{
    auto paths = GetPath();
    
    auto destPos = std::find_if(paths.begin(), paths.end(), [&dest](Point2D p) {
        return (dest.x == p.x && dest.y == p.y);
        });

    if (destPos != paths.end())
    {
        auto color = CollisionBoard::GetColorAt(dest.x, dest.y);
        
        //auto piece = nextPlayer->GetPieceAt(dest);
        //auto& pieces = nextPlayer->GetPieces();
        //// If the piece is not the same as that in the destination, it should be captured
        //// By implementation of GetPath(), any piece that is found on the path must be capturable
        //if (piece != pieces.end())
        //{
        //    if ((*piece)->name != CharacterName::KING)
        //        pieces.erase(piece);
        //}
        pos.x = dest.x;
        pos.y = dest.y;

        return true;
    }

    return false;
}



std::vector<Point2D> Pawn::GetPath()
{
    isFirstMove = startPos.x == pos.x && startPos.y == pos.y;
    std::vector<Point2D> v;
    int yDir = (isTop ? 1 : -1);
    int yStart = pos.y + yDir;

    int itCount = isFirstMove ? 2 : 1;

    Point2D p{ pos.x, 0 };
    for (int i = 0; i < itCount; i++)
    {
        p.y = yStart + (i * yDir);
        if (p.x < 0 || p.x >= CollisionBoard::COL_SIZE || p.y < 0 || p.y >= CollisionBoard::ROW_SIZE)
            continue;

        auto color = CollisionBoard::GetColorAt(p.x, p.y);
        if(color < 0)
            v.push_back(p);
    }

    // left
    p.y = yStart;
    for (int i = 0; i < 2; i++)
    {
        p.x = pos.x + (i == 0 ? 1: -1);
        if (p.x < 0 || p.x >= CollisionBoard::COL_SIZE || p.y < 0 || p.y >= CollisionBoard::ROW_SIZE)
            continue;
        auto color = CollisionBoard::GetColorAt(p.x, p.y);
        if(color >= 0 && color != GetColor())
            v.push_back(p);
    }

    return v;
}



std::vector<Point2D> Rook::GetPath()
{
    return Character::GetRookPath(*this);
}


std::vector<Point2D> Knight::GetPath()
{
    std::vector<Point2D> v;
    Point2D p;

    auto throughAxis = [&](const std::string& a = "y") {
        for (int i = 0; i < 2; i++)
        {

            auto axis1 = (a == "y") ? &p.y : &p.x;
            auto axis2 = (a == "y") ? &p.x : &p.y;

            auto& c_axis1 = (a == "y") ? pos.y : pos.x;
            auto& c_axis2 = (a == "y") ? pos.x : pos.y;

            *axis1 = c_axis1 + (i == 0 ? 2 : -2);

            for (int j = 0; j < 2; j++)
            {
                *axis2 = c_axis2 + (j == 0 ? 1 : -1);
                
                if (p.x < 0 || p.x >= CollisionBoard::COL_SIZE || p.y < 0 || p.y >= CollisionBoard::ROW_SIZE)
                    continue;

                auto color = CollisionBoard::GetColorAt(p.x, p.y);
                if (color < 0)
                    v.push_back(p);
                else
                {
                    if (!(GetColor() == color)) v.push_back(p);
                }
            }
        }
    };

    throughAxis("y");
    throughAxis("x");

    return v;
}


std::vector<Point2D> Bishop::GetPath()
{
   return GetBishopPath(*this);
}



Character::path_t Queen::GetPath()
{
    path_t v;

    auto path = GetRookPath(*this);
    v.insert(v.begin(), path.begin(), path.end());
    path = GetBishopPath(*this);
    v.insert(v.begin(), path.begin(), path.end());
    return v;
}


Character::path_t King::GetPath()
{
    path_t v, vc;

    vc.push_back({ pos.x, pos.y - 1 });
    vc.push_back({ pos.x - 1, pos.y - 1 });
    vc.push_back({ pos.x + 1, pos.y - 1 });

    vc.push_back({ pos.x - 1, pos.y });
    vc.push_back({ pos.x + 1, pos.y });

    vc.push_back({ pos.x, pos.y + 1 });
    vc.push_back({ pos.x - 1, pos.y + 1 });
    vc.push_back({ pos.x + 1, pos.y + 1 });

    for (auto it = vc.begin(); it != vc.end(); it++)
    {
        auto& p = *it;
        auto piece = getPieceAt(p);
        if (piece == characters.end() || !IsSame(**piece))
            v.push_back(p);
    }


    return v;
}


void processEvent(SDL_Event& evt)
{
    while (SDL_PollEvent(&evt))
    {
        if (evt.type == SDL_QUIT)
            canvas.windowShouldClose = true;
        if (evt.type == SDL_MOUSEBUTTONDOWN)
        {
            if (evt.button.button == SDL_BUTTON_LEFT)
            {
                int x = evt.button.x / CollisionBoard::TILE_SIZE;
                int y = evt.button.y / CollisionBoard::TILE_SIZE;

                auto name = CollisionBoard::GetNameAt(x, y);
                auto color = CollisionBoard::GetColorAt(x, y);

                if (!currentChr)
                {
                    if (currentPlayer->GetColor() == color)
                    {
                        auto selected = currentPlayer->GetPieceAt({ x, y });
                        // because of the color buffer, selected must always be a valid piece 
                        assert(selected != currentPlayer->GetPieces().end());
                        currentChr = &(**selected);
                    }
                }
                else
                {
                    if (currentChr->MoveTo({ x, y }))
                    {
                        currentChr = nullptr;
                        currentPlayer = (currentPlayer->GetColor() == 1 ? blackPlayer : whitePlayer);
                    }
                    else
                        currentChr = nullptr;
                }
            }
        }
    }
}


void mainLoop()
{
    while (!canvas.windowShouldClose)
    {
        update(1 / 60.0f);
        processEvent(canvas.evt);
        render(canvas.renderer);
    }
}



void initPlayers()
{
    bool player1IsWhite = true;
    currentPlayer = player1IsWhite ? &player1 : &player2;
    whitePlayer = player1IsWhite ? &player1 : &player2;
    blackPlayer = player1IsWhite ? &player2 : &player1;
    player1.Reset(player1IsWhite, false);
    player2.Reset(!player1IsWhite, true);

}



std::vector<std::unique_ptr<Character>>::iterator getPieceAt(Point2D pos)
{
    auto ind = std::find_if(characters.begin(), characters.end(), [&pos](auto& character) {
        auto& p = character->GetPos();
        return (p.x == pos.x && p.y == pos.y);
        });

    return ind;
}

void render(SDL_Renderer* renderer)
{
    SDL_RenderClear(renderer);
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);

    for (auto i = 0; i < ROW; i++)
    {
        for (auto j = 0; j < COL; j++)
        {
            auto s = (i * ROW + j);
            SDL_Texture* texture = nullptr;

            if (i % 2)
                texture = j % 2 ? textures["bg_dark_brown"] : textures["bg_light_brown"];
            else
                texture = j % 2 ? textures["bg_light_brown"] : textures["bg_dark_brown"];

            SDL_Rect dstRect{ j * TILESIZE, i * TILESIZE, TILESIZE, TILESIZE };
            SDL_RenderCopy(renderer, texture, nullptr, &dstRect);
        }
    }

    if (currentChr)
    {
        auto pos = currentChr->GetPos();
        SDL_Rect rect{ pos.x * TILESIZE, pos.y * TILESIZE, TILESIZE, TILESIZE };
        SDL_RenderCopy(renderer, textures["selected"], nullptr, &rect);

        auto visions = currentChr->GetPath();
        for (auto& vec : visions)
        {
            int sz = TILESIZE;
            int spacing = (TILESIZE - sz) / 2;
            SDL_Rect rect{ vec.x * TILESIZE + spacing, vec.y * TILESIZE + spacing, sz, sz };
            SDL_RenderCopy(renderer, textures["vision"], nullptr, &rect);
        }
    }

    player1.Render(renderer);
    player2.Render(renderer);

    SDL_RenderPresent(renderer);
}


void update(float dt)
{
    nextPlayer = currentPlayer == whitePlayer ? blackPlayer : whitePlayer;
    CollisionBoard::Reset();
    player1.Update();
    player2.Update();
}


void loadTexture(const std::string& name, const std::string& path)
{
    auto p = std::string("../../../assets/sprites/PNGs/With Shadow/2x/") + path;
    const char* _path = p.c_str();
    SDL_Surface* surface = IMG_Load(_path);
    if (!surface)
        SDL_Log("Failed to load image: %s", IMG_GetError());

    textures[name] = SDL_CreateTextureFromSurface(canvas.renderer, surface);
    SDL_FreeSurface(surface);
}


void loadTextures()
{
    loadTexture("bg_dark_brown", "square brown dark_2x.png");
    loadTexture("bg_light_brown", "square brown light_2x.png");
    //loadTexture("bg_dark_gray", "square gray dark_2x.png");
    //loadTexture("bg_light_gray", "square gray light_2x.png");
    loadTexture("w_pawn", "w_pawn_2x.png");
    loadTexture("b_pawn", "b_pawn_2x.png");
    loadTexture("b_rook", "b_rook_2x.png");
    loadTexture("w_rook", "w_rook_2x.png");
    loadTexture("w_bishop", "w_bishop_2x.png");
    loadTexture("b_bishop", "b_bishop_2x.png");
    loadTexture("w_knight", "w_knight_2x.png");
    loadTexture("b_knight", "b_knight_2x.png");
    loadTexture("w_king", "w_king_2x.png");
    loadTexture("b_king", "b_king_2x.png");
    loadTexture("w_queen", "w_queen_2x.png");
    loadTexture("b_queen", "b_queen_2x.png");
    loadTexture("selected", "selected.png");
    loadTexture("vision", "vision.png");
}


bool init()
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        std::cerr << "SDL2 Initialization failed: " << SDL_GetError() << std::endl;
        return false;
    }

    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG))
        SDL_Log("SDL_image init failed: %s", IMG_GetError());


    canvas.window = SDL_CreateWindow("Chess", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 512, 512, 0);
    if (!canvas.window)
    {
        std::cerr << "Unable to create SDL2 Window: " << SDL_GetError() << std::endl;
        return false;
    }

    canvas.renderer = SDL_CreateRenderer(canvas.window, -1, SDL_RENDERER_ACCELERATED);
    if (!canvas.renderer)
    {
        std::cerr << "Unable to create SDL2 Renderer: " << SDL_GetError() << std::endl;
        return false;
    }
    return true;
}


void CollisionBoard::Reset()
{
    nameBuffer.clear();
    colorBuffer.clear();

    for (size_t i = 0; i < ROW_SIZE; i++)
    {
        nameBuffer.push_back({ });
        colorBuffer.push_back({ });
        for (size_t j = 0; j < COL_SIZE; j++)
        {
            nameBuffer[i].push_back(CharacterName::NONE);
            colorBuffer[i].push_back(-1);
        }
    }
}


void CollisionBoard::SetPiece(Character& character)
{
    auto& pos = character.GetPos();
    nameBuffer[pos.y][pos.x] = character.GetName();
    colorBuffer[pos.y][pos.x] = character.isWhite ? 1 : 0;
}


void Player::Reset(bool _isWhite, bool isTop)
{
    pieces.clear();

    isWhite = _isWhite;
    int topOffset = isTop ? 1 : -1;
    int startY = isTop ? 0 : CollisionBoard::ROW_SIZE - 1;

    Point2D pos{ 0, startY };

    // make pawn
    for (int x = 0; x < CollisionBoard::COL_SIZE; x++)
    {
        pos.x = x;
        pos.y = startY + topOffset;
        pieces.push_back(std::make_unique<Pawn>(pos, isWhite, isTop));
    }

    // setup rook, knight, bishop
    pos.y = startY;

    for (int i = 0; i < 2; i++)
    {
        pos.x = i == 0 ? 0 : CollisionBoard::COL_SIZE - 1;
        pieces.push_back(std::make_unique<Rook>(pos, isWhite, isTop));

        pos.x = i == 0 ? 1 : CollisionBoard::COL_SIZE - 2;
        pieces.push_back(std::make_unique<Knight>(pos, isWhite, isTop));

        pos.x = i == 0 ? 2 : CollisionBoard::COL_SIZE - 3;
        pieces.push_back(std::make_unique<Bishop>(pos, isWhite, isTop));
    }

    // make queens
    pos.x = 3;
    pieces.push_back(std::make_unique<Queen>(pos, isWhite, isTop));

    // make kings
    pos.x = 4;
    pieces.push_back(std::make_unique<King>(pos, isWhite, isTop));

}


void Player::Update()
{
    for (const auto& piece : pieces)
        CollisionBoard::SetPiece(*piece);
}

void Player::Render(SDL_Renderer* renderer)
{
    for (const auto& piece : pieces)
        piece->Draw(renderer);
}


std::vector<std::unique_ptr<Character>>::iterator Player::GetPieceAt(Point2D pos)
{
    auto ind = std::find_if(pieces.begin(), pieces.end(), [&pos](auto& piece) {
        auto& p = piece->GetPos();
        return (p.x == pos.x && p.y == pos.y);
        });

    return ind;
}
