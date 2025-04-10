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
    initCharacters();

    mainLoop();

    return 0;
}


Character::Character(Point2D p, const std::string& _name, bool _isWhite, bool _isTop)
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
    if (name == "pawn")
        texture = textures[isWhite ? "w_pawn" : "b_pawn"];

    if (name == "rook")
        texture = textures[isWhite ? "w_rook" : "b_rook"];

    if (name == "knight")
        texture = textures[isWhite ? "w_knight" : "b_knight"];

    if (name == "bishop")
        texture = textures[isWhite ? "w_bishop" : "b_bishop"];

    if (name == "king")
        texture = textures[isWhite ? "w_king" : "b_king"];

    if (name == "queen")
        texture = textures[isWhite ? "w_queen" : "b_queen"];

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



std::vector<Point2D> Pawn::GetPath()
{
    isFirstMove = startPos.x == pos.x && startPos.y == pos.y;
    std::vector<Point2D> v;
    int yOffset = (isTop ? 1 : -1);
    int yStart = pos.y + yOffset;

    // forward
    Point2D p{ pos.x, yStart };
    auto piece = getPieceAt(p);
    if (piece == characters.end())
    {
        v.push_back(p);
        if (isFirstMove)
        {
            p.y += yOffset;
            piece = getPieceAt(p);
            if (piece == characters.end())
                v.push_back(p);
        }
    }

    // left
    for (int i = 0; i < 2; i++)
    {
        p.x = pos.x + (i == 0 ? 1: -1);
        piece = getPieceAt(p);
        if (piece != characters.end())
        {
            if (isWhite && !(*piece)->isWhite)
                v.push_back(p);
            if (!isWhite && (*piece)->isWhite)
                v.push_back(p);
        }
    }

    return v;
}



std::vector<Point2D> Rook::GetPath()
{
    std::vector<Point2D> v;

    // up and down
    Point2D p{ pos.x, pos.y };

    for (int i = 0; i < 2; i++)
    {
        p.x = pos.x;
        p.y = pos.y;

        bool isHit = false;
        int dir = i == 0 ? 1 : -1;
        p.y += dir;
        while (!isHit && p.y >= 0 && p.y < ROW)
        {
            auto piece = getPieceAt(p);
            if (piece == characters.end())
                v.push_back(p);
            else if (piece != characters.end())
            {
                if(!IsSame(**piece)) v.push_back(p);
                isHit = true;
            }
            p.y += dir;
        }
    }

    // for x
    for (int i = 0; i < 2; i++)
    {
        p.x = pos.x;
        p.y = pos.y;

        bool isHit = false;
        int dir = i == 0 ? 1 : -1;
        p.x += dir;
        while (!isHit && p.x >= 0 && p.x < ROW)
        {
            auto piece = getPieceAt(p);
            if (piece == characters.end())
                v.push_back(p);
            else if (piece != characters.end())
            {
                if (!IsSame(**piece)) v.push_back(p);
                isHit = true;
            }
            p.x += dir;
        }
    }

    return v;
}


std::vector<Point2D> Knight::GetPath()
{
    std::vector<Point2D> v;
    Point2D p;

    for (int i = 0; i < 2; i++)
    {
        p.y = pos.y + (i == 0 ? 2 : -2);

        for (int j = 0; j < 2; j++)
        {
            p.x = pos.x + (j == 0 ? 1 : -1);
            auto piece = getPieceAt(p);
            if (piece == characters.end() || !IsSame(**piece))
                v.push_back(p);
        }
    }


    for (int i = 0; i < 2; i++)
    {
        p.x = pos.x + (i == 0 ? 2 : -2);

        for (int j = 0; j < 2; j++)
        {
            p.y = pos.y + (j == 0 ? 1 : -1);
            auto piece = getPieceAt(p);
            if (piece == characters.end() || !IsSame(**piece))
                v.push_back(p);
        }
    }

    return v;
}


std::vector<Point2D> Bishop::GetPath()
{
    std::vector<Point2D> v;

    Point2D p{ pos.x, pos.y };

    // for top
    for (int i = 0; i < 2; i++)
    {
        int dir = i == 0 ? -1 : 1;
        bool isHit = false;
        p.x = pos.x;
        p.y = pos.y;
        p.x += dir;
        p.y -= 1;

        while (!isHit && p.x >= 0 && p.x < COL && p.y >= 0)
        {
            auto piece = getPieceAt(p);
            if (piece == characters.end())
                v.push_back(p);
            else if (piece != characters.end())
            {
                if (!IsSame(**piece)) v.push_back(p);
                isHit = true;
            }

            p.x += dir;
            p.y -= 1;
        }

    }
        
    // for down
    for (int i = 0; i < 2; i++)
    {
        int dir = i == 0 ? -1 : 1;
        bool isHit = false;
        p.x = pos.x;
        p.y = pos.y;
        p.x += dir;
        p.y += 1;

        while (!isHit && p.x >= 0 && p.x < COL && p.y >= 0)
        {
            auto piece = getPieceAt(p);
            if (piece == characters.end())
                v.push_back(p);
            else if (piece != characters.end())
            {
                if (!IsSame(**piece)) v.push_back(p);
                isHit = true;
            }

            p.x += dir;
            p.y += 1;
        }

    }

    return v;
}


class Queen : public Character
{
public:
    Queen(Point2D p, bool isWhite, bool isTop) : Character(p, "queen", isWhite, isTop) {};
    
    std::vector<Point2D> GetPath()
    {
        return {};
    }
};


class King : public Character
{
public:
    King(Point2D p, bool isWhite, bool isTop) : Character(p, "king", isWhite, isTop) {};
    
    std::vector<Point2D> GetPath()
    {
        return {};
    }
};


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
                int x = evt.button.x / TILESIZE;
                int y = evt.button.y / TILESIZE;

                if (!currentChr)
                {
                    auto selected = std::find_if(characters.begin(), characters.end(), [&x, &y](auto& character) {
                        auto p = character->GetPos();
                        return p.x == x && p.y == y;
                        });

                    if (selected != characters.end())
                    {
                        currentChr = &(**selected);
                    }
                } else {
                    currentChr->MoveTo({ x, y });
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
        processEvent(canvas.evt);
        update(1 / 60.0f);
        render(canvas.renderer);
    }
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

    for (auto& character : characters)
        character->Draw(renderer);

    SDL_RenderPresent(renderer);
}


void update(float dt)
{
}

void initCharacters()
{
    characters.clear();

    // i for both top and bottom
    for (int i = 0; i < 2; i++)
    {
        bool isTop = (i == 0);
        bool isWhite = (i == 0);

        int topOffset = isTop ? 1 : -1;
        int startY = isTop ? 0 : ROW - 1;

        Point2D pos{ 0, startY };

        // make pawn
        for (int x = 0; x < COL; x++)
        {
            pos.x = x;
            pos.y = startY + topOffset;
            characters.push_back(std::make_unique<Pawn>(pos, isWhite, isTop));
        }

        // setup rook, knight, bishop
        pos.y = startY;

        for (int i = 0; i < 2; i++)
        {
            pos.x = i == 0 ? 0 : COL - 1;
            characters.push_back(std::make_unique<Rook>(pos, isWhite, isTop));

            pos.x = i == 0 ? 1 : COL - 2;
            characters.push_back(std::make_unique<Knight>(pos, isWhite, isTop));

            pos.x = i == 0 ? 2 : COL - 3;
            characters.push_back(std::make_unique<Bishop>(pos, isWhite, isTop));
        }

        // make queens
        pos.x = 3;
        characters.push_back(std::make_unique<Queen>(pos, isWhite, isTop));
        
        // make kings
        pos.x = 4;
        characters.push_back(std::make_unique<King>(pos, isWhite, isTop));
    }
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