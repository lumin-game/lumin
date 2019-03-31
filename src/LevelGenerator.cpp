#include "LevelGenerator.hpp"
#include "entity.hpp"
#include "door.hpp"
#include "movable_wall.hpp"
#include "common.hpp"
#include "switch.hpp"
#include "firefly.hpp"
#include "glass.hpp"
#include "fog.hpp"
#include "lantern.hpp"

#include <iostream>
#include <string.h>
#include <sstream>

#define BLOCK_SIZE 64

std::map<char, StaticTile> LevelGenerator::tile_map = {
        {'#', WALL},
        {'$', GLASS},
        {'+', DARKWALL},
        {'-', LIGHTWALL},
        {'~', FOG},
        {'*', FIREFLY},
        {'&', PLAYER}
};

void LevelGenerator::create_current_level(int level, Player& outPlayer, std::vector<Entity*>& outEntities) {
    std::ifstream in(levels_path("level_" + std::to_string(level) + ".txt"));

    if (!in) {
        std::cerr << "Cannot open file. \n" << std::endl;
        return;
    }

    std::vector<std::vector<char>> grid;
    std::map<char, std::pair<int, int>> dynamicEntityLocs;
    std::map<char, Entity*> dynamicEntities;

    std::string row;
    int y = 0;

    while (std::getline(in, row)) {
        std::vector<char> charVector(row.begin(), row.end());

        // Ignore empty lines in the level file
        if (!charVector.empty()) {

            if (charVector[0] == '?') {
                // Parse entity declaration
                const char name = charVector[1];
                const char type = charVector[2];

                Entity* entity;

                switch (type) {
                    // Switch
                    case '/':
                        entity = new Switch();
                        break;

                        // Moving platform
                    case '_':
                        entity = new MovableWall();
                        break;

                    case '|':
                        entity = new Door();
                        // Make default state of door open; if we later link it to a switch,
                        // we turn its default state to off as part of the linking process.
                        entity->set_lit(true);
                        break;
                    case '@':
                        entity = new Lantern();
                        break;

                    default:
                        fprintf(stderr, "Unknown entity declaration in level file: %c: %c\n", name, type);
                        continue;
                }

                if (dynamicEntityLocs.find(name) == dynamicEntityLocs.end())
                {
                    continue;
                }
                std::pair<int, int> coord = dynamicEntityLocs.find(name)->second;
                entity->init(coord.first * BLOCK_SIZE, coord.second * BLOCK_SIZE);
                dynamicEntities.insert(std::pair<char, Entity*>(name, entity));
                outEntities.push_back(entity);
            }
            else if (charVector[0] == '=') {
                // Parse entity relationship
                auto entity1 = dynamicEntities.find(charVector[1]);
                auto entity2 = dynamicEntities.find(charVector[2]);

                if (entity1 == dynamicEntities.end()) {
                    fprintf(stderr, "Couldn't parse first entity in relationship: %c\n", charVector[1]);
                    continue;
                }

                if (entity2 == dynamicEntities.end()) {
                    fprintf(stderr, "Couldn't parse second entity in relationship: %c\n", charVector[2]);
                    continue;
                }

                if (!entity1->second || !entity2->second) {
                    continue;
                }

                (entity1->second)->register_entity(entity2->second);

                // Door logic!
                if (Door *door = dynamic_cast<Door *>(entity2->second)) {
                    door->set_lit(false);
                }

            }
            else if (charVector[0] == '@') {
                // Parse entity property declaration
                const char name = charVector[1];
                auto entity = dynamicEntities.find(name);

                if (entity == dynamicEntities.end()) {
                    fprintf(stderr, "Couldn't set property for entity '%c'\n", name);
                    continue;
                }

                if (!entity->second) {
                    continue;
                }

                // Switch property declaration
                if (auto *s = dynamic_cast<Switch *>(entity->second)) {
                   if (charVector[2] == 'T') {
                       s->set_toggle_switch(true);
                   }

                // Moving platform movement declaration
				} else if (MovableWall *mw = dynamic_cast<MovableWall*>(entity->second)) {

                    if (dynamicEntityLocs.find(name) == dynamicEntityLocs.end())
                    {
                        continue;
                    }
                    std::pair<int, int> start = dynamicEntityLocs.find(name)->second;
                    vec2 initialBlockLocation = { (float) start.first, (float) start.second };

                    row.erase(0, row.find(" ") + 1);

                    bool moveImmediate = row.find("M") < row.size();
                    bool loopMovement = row.find("L") < row.size();;
                    bool reverseOnLoop = row.find("R") < row.size();;
                    bool shouldCurve = false;

                    std::vector<vec2> blockLocations;
                    std::vector<vec2> blockCurves;

                    std::string curve = "~";
                    std::string openParen = "(";
                    std::string closeParen = ")";

                    int curveInd = row.find(curve);
                    if (curveInd < row.size())
                    {
                        shouldCurve = true;
                        std::string curveDefinition = row.substr(curveInd, row.size() - curveInd);
                        row.erase(curveInd, row.size());

                        int index = curveDefinition.find(openParen);
                        while (index < curveDefinition.size())
                        {
                            int end = curveDefinition.find(closeParen);
                            if (end >= curveDefinition.size())
                            {
                                fprintf(stderr, "Syntax malformat in MovableWall path declaration!");
                                continue;
                            }

                            std::string coord = curveDefinition.substr(index + 1, end - index - 1);
                            int comma = coord.find(",");
                            std::string xBlockStr = coord.substr(0, comma);
                            std::string yBlockStr = coord.substr(comma + 1, coord.size() - comma);

                            int xBlock = stoi(xBlockStr);
                            int yBlock = stoi(yBlockStr);

                            blockCurves.push_back(initialBlockLocation + vec2({ (float)xBlock, (float)yBlock }));

                            curveDefinition.erase(0, end + 1);
                            index = curveDefinition.find(openParen);
                        }
                    }

                    int index = row.find(openParen);
                    while (index < row.size())
                    {
                        int end = row.find(closeParen);
                        if (end >= row.size())
                        {
                            fprintf(stderr, "Syntax malformat in MovableWall path declaration!");
                            continue;
                        }

                        std::string coord = row.substr(index + 1, end - index - 1);
                        int comma = coord.find(",");
                        std::string xBlockStr = coord.substr(0, comma);
                        std::string yBlockStr = coord.substr(comma + 1, coord.size() - comma);

                        int xBlock = stoi(xBlockStr);
                        int yBlock = stoi(yBlockStr);

                        blockLocations.push_back(initialBlockLocation + vec2({ (float)xBlock, (float)yBlock }));

                        row.erase(0, end + 1);
                        index = row.find(openParen);
                    }

                    // TODO: map different movement types to the 4th character in the declaration
                    mw->set_movement_properties(shouldCurve, blockLocations, blockCurves, 0.2, moveImmediate, loopMovement, reverseOnLoop);
                }

				if (Door *door = dynamic_cast<Door *>(entity->second)) {
					int level = 0;
					for (int i = 2; i < 4; i++) {
						level *= 10;
						level += charVector[i] - '0';
					}
					door->set_level_index(level);
				}
            }
            else {
                // Keep track of dynamic dynamicEntities
                for (int x = 0; x < charVector.size(); x++) {
                    const char c = charVector[x];
                    if (('0' <= c && c <= '9') || ('A' <= c && c <= 'Z')) {
                        const std::pair<int, int> coord = std::make_pair(x, y);
                        dynamicEntityLocs.insert(std::pair<char, std::pair<int, int>>(c, coord));
                    }
                }

                // Push entire row into grid vector
                grid.push_back(charVector);
                y++;
            }
        }
    }

    in.close();
    create_level(grid, outPlayer, outEntities);
}

bool LevelGenerator::add_tile(int x_pos, int y_pos, StaticTile tile, Player& outPlayer, std::vector<CreatedEntity>& outCreateEntities) {
    Entity *level_entity = nullptr;

    switch (tile) {
        case WALL:
            level_entity = createTile<Wall>(x_pos, y_pos);
            break;
        case GLASS:
            level_entity = createTile<Glass>(x_pos, y_pos);
            break;
        case DARKWALL:
            // TODO: add dark wall entity
            break;
        case LIGHTWALL:
            // TODO: add light wall entity
            break;
        case FOG:
            level_entity = createTile<Fog>(x_pos, y_pos);
            break;
        case FIREFLY:
            level_entity = createTile<Firefly>(x_pos, y_pos);
            break;
        case PLAYER:
            outPlayer.init();
            // spawn player 1 tile higher to ensure that the player doesn't fall
            outPlayer.setPlayerPosition({ (float)x_pos * BLOCK_SIZE, (float)(y_pos - 1) * BLOCK_SIZE });
            return true;
    }

    if (!level_entity) {
        fprintf(stderr, "Level entity is not set \n");
		return false;
    }

	CreatedEntity createdEntity;
	createdEntity.x = x_pos;
	createdEntity.y = y_pos;
	createdEntity.entity = level_entity;

	outCreateEntities.push_back(createdEntity);
	return true;
}

template <class TEntity>
TEntity* LevelGenerator::createTile(int x_pos, int y_pos)
{
    TEntity* entity = new TEntity();
    entity->init(x_pos * BLOCK_SIZE, y_pos * BLOCK_SIZE);
    return entity;
}

// Just to print the grid (testing purposes)
void LevelGenerator::print_grid(std::vector<std::vector<char>>& grid) {
    for (std::vector<char> row : grid) {
        for (char cell : row) {
            std::cout << cell << " ";
        }
        std::cout << std::endl;
    }
}

void LevelGenerator::create_level(std::vector<std::vector<char>>& grid, Player& outPlayer, std::vector<Entity*>& outEntities) {
	std::vector<CreatedEntity> createdEntities;
	for (int y = 0; y < grid.size(); y++) {
        for (int x = 0; x < grid[y].size(); x++) {
            auto tile = tile_map.find(grid[y][x]);
			if (tile != tile_map.end()) {
				add_tile(x, y, tile->second, outPlayer, createdEntities);
			}
        }
    }

	for (const CreatedEntity& createdEntity : createdEntities)
	{
		Fog* fogTile = dynamic_cast<Fog*>(createdEntity.entity);
		if (fogTile)
		{
			for (const CreatedEntity& otherEntity : createdEntities)
			{
				if ((otherEntity.x == createdEntity.x - 1 && otherEntity.y == createdEntity.y)
					|| (otherEntity.x == createdEntity.x && otherEntity.y == createdEntity.y - 1))
				{
					Fog* neighborFogTile = dynamic_cast<Fog*>(otherEntity.entity);
					if (neighborFogTile)
					{
						if (otherEntity.x == createdEntity.x - 1)
						{
							fogTile->GetNeighborFogStruct().left = true;
							neighborFogTile->GetNeighborFogStruct().right = true;
						}
						else
						{
							fogTile->GetNeighborFogStruct().bottom = true;
							neighborFogTile->GetNeighborFogStruct().top = true;
						}
					}
				}
			}
		}
	}

	for (const CreatedEntity& createdEntity : createdEntities)
	{
		outEntities.push_back(createdEntity.entity);
	}
}