#ifndef SYSTEM_HPP
#define SYSTEM_HPP
#include "game.hpp"
#include <vector>
#include <iostream>
#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <SFML/Window.hpp>

using namespace G;

class QuadTree {
	private:
		int MAX_ENTITIES = 8;
		int MAX_LEVELS = 5;
		int level;
		std::vector<EntityId> entities;
		std::vector<QuadTree *> nodes;
		sf::Rect<float> bounds;
	public:
		QuadTree(sf::Rect<float> b, int l) :nodes(4, nullptr), bounds(b), level(l) {
		};
		
		void clear() {
			entities.clear();
			for(int i = 0; i < 4; i++) {
				if(nodes[i] != nullptr) {
					delete nodes[i];
				}
			}
			nodes.clear();
		};
		
		void split() {
			float subW = bounds.width/2;
			float subH = bounds.height/2;
			float x = bounds.left;
			float y = bounds.top;
			
			nodes[0] = new QuadTree(sf::Rect<float>(x, y, subW, subH), level+1);
			nodes[1] = new QuadTree(sf::Rect<float>(x+subW, y, subW, subH), level+1);
			nodes[2] = new QuadTree(sf::Rect<float>(x, y+subH, subW, subH), level+1);
			nodes[3] = new QuadTree(sf::Rect<float>(x+subW, y+subH, subW, subH), level+1);
		};
		
		int getIndex(HitboxComponent& rect) {
			int index = -1;
			float vMid = bounds.left + bounds.width/2;
			float hMid = bounds.top + bounds.height/2;
			
			bool topQ = (rect.position->y < hMid && rect.position->y+rect.height < hMid);
			bool bottomQ = (rect.position->y > hMid);
			
			if(rect.position->x < vMid && rect.position->x+rect.width < vMid) {
				if(topQ) {
					index = 0;
				} else if(bottomQ) {
					index = 2;
				}
			}
			if(rect.position->x > vMid) {
				if(topQ) {
					index = 1;
				} else if (bottomQ) {
					index = 3;
				}
			}
			
			return index;
		};
		
		int getIndex(float x, float y) {
			int index = -1;
			float vMid = bounds.left + bounds.width/2;
			float hMid = bounds.top + bounds.height/2;
			
			bool topQ = (y < hMid);
			bool bottomQ = (y > hMid);
			
			if(x < vMid) {
				if(topQ) {
					index = 0;
				} else if(bottomQ) {
					index = 2;
				}
			}
			if(x > vMid) {
				if(topQ) {
					index = 1;
				} else if (bottomQ) {
					index = 3;
				}
			}
			
			return index;
		};
		
		void insert(System<HitboxComponent>& hbSys, EntityId e) {
			HitboxComponent rect = (*hbSys.components[e]);
			if(nodes[0] != nullptr) {
				int index = getIndex(rect);
			
				if(index != -1) {
					nodes[index]->insert(hbSys, e);
					return;
				}
			}
			//std::cout << "ADDING: " << e << "\n";
			entities.push_back(rect.owner);
			
			if(entities.size() > MAX_ENTITIES && level < MAX_LEVELS) {
				
				if(nodes[0] == nullptr) {
					split();
				}
				/* for(std::vector<EntityId>::iterator it = entities.begin(); it != entities.end();) {
					int index = getIndex((*hbSys.components[*it]));
					//std::cout << " ENTITY: " << *it << "INDEX: " << index << "\n";
					if(index != -1 && *it != 0) {
						nodes[index]->insert(hbSys, *it);
						entities.erase(it);
					} else {
						++it;
					}
				} */
				for(int i = 0; i < entities.size(); i++)
				{
					int index = getIndex((*hbSys.components[entities[i]]));
					//std::cout << " ENTITY: " << entities[i] << " INDEX: " << index << " LEVEL: " << level <<" SIZE: " << entities.size() << "\n";
					if(index != -1 && entities[i] != 0) {
						nodes[index]->insert(hbSys, entities[i]);
						entities.erase(entities.begin()+i);
						--i;
					}
				}
				//entities.resize(cSize - removed);
			
			}
			//std::cout << "ENTITIES SIZE: " << entities.size() << " LEVEL: " << level << "\n";
		};
		
		std::vector<EntityId> retreive(std::vector<EntityId>& returnEntities, HitboxComponent& rect) {
			int index = getIndex(rect);
			std::vector<EntityId> holder;
			if(index != -1 && nodes[0] != nullptr) {
				nodes[index]->retreive(returnEntities, rect);
			} else if (index == -1 && nodes[0] != nullptr)
			{
				if(getIndex(rect.position->x, rect.position->y) == 0 && getIndex(rect.position->x+rect.width, rect.position->y+rect.height) == 3) {
					nodes[getIndex(rect.position->x, rect.position->y)]->retreive(returnEntities, rect);
					nodes[getIndex(rect.position->x+rect.width, rect.position->y)]->retreive(returnEntities, rect);
					nodes[getIndex(rect.position->x, rect.position->y+rect.height)]->retreive(returnEntities, rect);
					nodes[getIndex(rect.position->x+rect.width, rect.position->y+rect.height)]->retreive(returnEntities, rect);
				}
				if(getIndex(rect.position->x, rect.position->y) == 0 && getIndex(rect.position->x+rect.width, rect.position->y+rect.height) == 2) {
					nodes[getIndex(rect.position->x, rect.position->y)]->retreive(returnEntities, rect);
					nodes[getIndex(rect.position->x, rect.position->y+rect.height)]->retreive(returnEntities, rect);
				}
				if(getIndex(rect.position->x, rect.position->y) == 0 && getIndex(rect.position->x+rect.width, rect.position->y+rect.height) == 1) {
					nodes[getIndex(rect.position->x, rect.position->y)]->retreive(returnEntities, rect);
					nodes[getIndex(rect.position->x+rect.width, rect.position->y)]->retreive(returnEntities, rect);
				}
				if(getIndex(rect.position->x, rect.position->y) == 2 && getIndex(rect.position->x+rect.width, rect.position->y+rect.height) == 3) {
					nodes[getIndex(rect.position->x, rect.position->y+rect.height)]->retreive(returnEntities, rect);
					nodes[getIndex(rect.position->x+rect.width, rect.position->y+rect.height)]->retreive(returnEntities, rect);
				}
				if(getIndex(rect.position->x, rect.position->y) == 1 && getIndex(rect.position->x+rect.width, rect.position->y+rect.height) == 3) {
					nodes[getIndex(rect.position->x+rect.width, rect.position->y)]->retreive(returnEntities, rect);
					nodes[getIndex(rect.position->x+rect.width, rect.position->y+rect.height)]->retreive(returnEntities, rect);
				}
				
			}
			//returnEntities.insert(returnEntities.end(), entities.begin(), entities.end());
			for(int i = 0; i < entities.size(); i++) {
				returnEntities.push_back(entities[i]);
				//std::cout << entities[i] << "\n";
			}
			/* std::cout <<"LEVEL: " << level << "\n"; */
			/* for(int i = 0; i < returnEntities.size(); i++) {
				std::cout << returnEntities[i] << "\n";
			} */
			return returnEntities;
		};
		
		void render(sf::RenderWindow& window) {
			if(nodes[0] != nullptr){
				sf::RectangleShape vL(sf::Vector2f(bounds.width, 1));
				sf::RectangleShape hL(sf::Vector2f(1, bounds.height));
				vL.setFillColor(sf::Color::White);
				hL.setFillColor(sf::Color::White);
				vL.setPosition(bounds.left, bounds.top);
				window.draw(vL);
				vL.setPosition(bounds.left, bounds.top+(bounds.height/2));
				window.draw(vL);
				vL.setPosition(bounds.left, bounds.top+(bounds.height));
				window.draw(vL);
				hL.setPosition(bounds.left, bounds.top);
				window.draw(hL);
				hL.setPosition(bounds.left+(bounds.width/2), bounds.top);
				window.draw(hL);
				hL.setPosition(bounds.left+(bounds.width), bounds.top);
				window.draw(hL);
			}
			if(nodes[0] != nullptr)
			{
				for(int i = 0; i < 4; i++) {
					nodes[i]->render(window);
				}
			}
			//r.setOutlineColor(sf::Color::White);
			//r.setOutlineThickness(10);
			//r.setFillColor(sf::Color::White);
			
			//window.draw(r);
		}
		
};

bool overlap(HitboxComponent o1, HitboxComponent o2) {
	sf::Rect<float> r1(o1.position->x, o1.position->y, o1.width, o1.height);
	sf::Rect<float> r2(o2.position->x, o2.position->y, o2.width, o2.height);
	return r1.intersects(r2);
};

bool collide(EntityId e1, EntityId e2, World& world) {
	float vx1 = world.mSys.components[e1]->vx;
	float vx2 = world.mSys.components[e2]->vx;
	if(vx1 > 0)
	{
		world.pSys.components[e1]->x = world.pSys.components[e2]->x - world.hbSys.components[e1]->width;
	} else if(vx1 < 0) {
		world.pSys.components[e1]->x = world.pSys.components[e2]->x + world.hbSys.components[e2]->width;
	} else if(vx1 == 0) {
		if(vx2 < 0)
		{
			world.pSys.components[e1]->x = world.pSys.components[e2]->x - world.hbSys.components[e1]->width;
		} else if(vx2 > 0) {
			world.pSys.components[e1]->x = world.pSys.components[e2]->x + world.hbSys.components[e2]->width;
		}
	}
	world.mSys.components[e1]->vx = 0;
	world.mSys.components[e2]->vx = 0;
	
};

#endif