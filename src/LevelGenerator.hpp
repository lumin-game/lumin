#pragma once

#include <vector>
#include <map>
#include "entity.hpp"


class LevelGenerator
{
	struct CreatedEntity
	{
		int x;
		int y;
		Entity* entity;
	};

public:
	LevelGenerator() = default;

	void create_current_level(int level, Player& outPlayer, std::vector<Entity*>& outEntities);

private:
	void create_level(std::vector<std::vector<char>>& grid, Player& outPlayer, std::vector<Entity*>& outEntities);

	bool add_tile(int x_pos, int y_pos, StaticTile tile, Player& outPlayer, std::vector<CreatedEntity>& outCreateEntities);

	void print_grid(std::vector<std::vector<char>>& grid);

	template <class TEntity>
	TEntity* createTile(int x_pos, int y_pos);

private:
	static std::map<char, StaticTile> tile_map;
};