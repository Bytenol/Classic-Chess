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

#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>


// forward classes declaration
struct Point2D;
class Character;

extern Character* currentChr;
extern std::vector<std::unique_ptr<Character>> characters;
extern std::map<std::string, SDL_Texture*> textures;


std::vector<std::unique_ptr<Character>>::iterator getPieceAt(Point2D pos);

void render(SDL_Renderer* renderer);

void update(float dt);

void processEvent(SDL_Event& evt);

void mainLoop();

void initCharacters();

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
    std::string name;

    bool isFirstMove = true;
    Point2D startPos;

public:
    bool isWhite = false;

    Character(Point2D p, const std::string& _name, bool _isWhite, bool _isTop);

    void Draw(SDL_Renderer* renderer);

    void SetPos(int x, int y);

    decltype(pos)& GetPos();

    bool IsSame(const Character& other);


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
    virtual void MoveTo(Point2D dest)
    {
        auto n = GetPath();
        auto isMoveable = std::find_if(n.begin(), n.end(), [&dest](Point2D p) {
            return (dest.x == p.x && dest.y == p.y);
            });

        if (isMoveable != n.end())
        {
            auto piece = getPieceAt(dest);

            // If the piece is not the same as that in the destination, it should be captured
            // By implementation of GetPath(), any piece that is found on the path must be capturable
            if (piece != characters.end())
            {
                characters.erase(piece);
                std::cout << "Implement points based on character here: " << std::endl;
            }
            pos.x = dest.x;
            pos.y = dest.y;
        }
    }

};


class Pawn: public Character
{

public:
    Pawn(Point2D p, bool isWhite, bool isTop) : Character(p, "pawn", isWhite, isTop) {};
    std::vector<Point2D> GetPath();
};


class Rook : public Character
{
public:
    Rook(Point2D p, bool isWhite, bool isTop) : Character(p, "rook", isWhite, isTop) {};

    std::vector<Point2D> GetPath();
};


class Knight : public Character
{
public:
    Knight(Point2D p, bool isWhite, bool isTop) : Character(p, "knight", isWhite, isTop) {};

    std::vector<Point2D> GetPath();
};


class Bishop : public Character
{
public:
    Bishop(Point2D p, bool isWhite, bool isTop) : Character(p, "bishop", isWhite, isTop) {};

    std::vector<Point2D> GetPath();
};


#endif 
