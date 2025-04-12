/*****************************************************************//**
 * \file   main.hpp
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
#pragma once
#ifndef __BYTENOL_CHESS_MAIN_HPP__
#define __BYTENOL_CHESS_MAIN_HPP__

#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <memory>
#include <map>
#include <cassert>

#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>


// forward classes declaration
struct Point2D;
class Character;
class Player;
class CollisionBoard;

enum class CharacterName
{
    NONE,
    PAWN,
    ROOK,
    KNIGHT,
    BISHOP,
    KING,
    QUEEN
};

extern Character* currentChr;
extern std::vector<std::unique_ptr<Character>> characters;
extern std::map<std::string, SDL_Texture*> textures;
extern Player player1, player2;


std::vector<std::unique_ptr<Character>>::iterator getPieceAt(Point2D pos);

void render(SDL_Renderer* renderer);

void update(float dt);

void processEvent(SDL_Event& evt);

void mainLoop();

void initPlayers();

void loadTexture(const std::string& name, const std::string& path);

void loadTextures();

bool init();


struct Point2D
{
    int x, y;
};


class Character
{

protected:
    Point2D pos{ 0, 0 };
    Point2D oldPos{ 0, 0 };
    bool isTop = false;
    bool isSelected = false;
    CharacterName name;

    bool isFirstMove = true;
    Point2D startPos;

    using path_t = std::vector<Point2D>;

public:
    bool isWhite = false;

    Character(Point2D p, CharacterName _name, bool _isWhite, bool _isTop);

    void Draw(SDL_Renderer* renderer);

    void SetPos(int x, int y);

    decltype(pos)& GetPos();

    bool IsSame(const Character& other);

    static path_t GetRookPath(Character& character);

    static path_t GetBishopPath(Character& character);


    /**
     * You should define the path logic for each piece.
     * By convention, when an enemy is obstructing a path, their position should
     * be registered as path of the path and must be the end to that path.
     * 
     * This will allow the default Move() function to allow capturing that such enemy
     */
    virtual std::vector<Point2D> GetPath() = 0;


    /**
     * This method implements the movement logic for the game
     * @param dest is the destination to move the piece to
     * 
     */
    virtual bool MoveTo(Point2D dest);

    inline CharacterName GetName() const
    {
        return name;
    }

    inline int GetColor() const
    {
        return isWhite ? 1 : 0;
    }

};


class Pawn: public Character
{

public:
    Pawn(Point2D p, bool isWhite, bool isTop) : Character(p, CharacterName::PAWN, isWhite, isTop) {};
    std::vector<Point2D> GetPath();
};


class Rook : public Character
{
public:
    Rook(Point2D p, bool isWhite, bool isTop) : Character(p, CharacterName::ROOK, isWhite, isTop) {};

    std::vector<Point2D> GetPath();
};


class Knight : public Character
{
public:
    Knight(Point2D p, bool isWhite, bool isTop) : Character(p, CharacterName::KNIGHT, isWhite, isTop) {};

    std::vector<Point2D> GetPath();
};


class Bishop : public Character
{
public:
    Bishop(Point2D p, bool isWhite, bool isTop) : Character(p, CharacterName::BISHOP, isWhite, isTop) {};

    std::vector<Point2D> GetPath();
};


class Queen : public Character
{
public:
    Queen(Point2D p, bool isWhite, bool isTop) : Character(p, CharacterName::QUEEN, isWhite, isTop) {};

    path_t GetPath();

};


class King : public Character
{
public:
    King(Point2D p, bool isWhite, bool isTop) : Character(p, CharacterName::KING, isWhite, isTop) {};

    path_t GetPath();
};



class Player
{

    std::vector<std::unique_ptr<Character>> pieces;

public:
    bool isWhite = false;
    int score = 0;
    Player() = default;

    void Reset(bool _isWhite, bool isTop);

    void Update();

    void Render(SDL_Renderer* renderer);

    std::vector<std::unique_ptr<Character>>::iterator GetPieceAt(Point2D pos);

    inline decltype(pieces)& GetPieces()
    {
        return pieces;
    };

    inline int GetColor() const
    {
        return isWhite ? 1 : 0;
    }
};


class CollisionBoard
{
    static std::vector<std::vector<CharacterName>> nameBuffer;
    static std::vector<std::vector<int>> colorBuffer;

public:
    static const size_t COL_SIZE = 8;
    static const size_t ROW_SIZE = 8;
    static const size_t TILE_SIZE = 64;

    static void Reset();

    static void SetPiece(Character& character);

    static inline CharacterName GetNameAt(int x, int y)
    {
        return nameBuffer[y][x];
    }

    static inline int GetColorAt(int x, int y)
    {
        return colorBuffer[y][x];
    }
};

#endif 
