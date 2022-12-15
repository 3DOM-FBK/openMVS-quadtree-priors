/*
* QuadTree.h
*
* Copyright (c) 2021 FBK-3DOM
*
* Author(s):
*
*      
*
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU Affero General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU Affero General Public License for more details.
*
* You should have received a copy of the GNU Affero General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
*
* Additional Terms:
*
*      You are required to preserve legal notices and author attributions in
*      that material or in the Appropriate Legal Notices displayed by works
*      containing it.
*/

#ifndef _MVS_QUADTREE_H_
#define _MVS_QUADTREE_H_

#include "Types.h"

namespace FBK3DOM 
{
	class QuadTreeNode
	{

	public:
		short x, y;
		short width, height;
		int id;
		int level;
		cv::Vec3b avgColor;
		//cv::Vec3b abColor;
		int color;


		QuadTreeNode() {}
		QuadTreeNode(int id, short x, short y, short width, short height, int level, std::shared_ptr<QuadTreeNode> parent, int color)
			: id(id), x(x), y(y), width(width), height(height), level(level), avgColor(cv::Vec3b(0, 0, 0)), parent(parent), color(color) {}

		std::shared_ptr<QuadTreeNode> top;
		std::shared_ptr<QuadTreeNode> left;
		std::shared_ptr<QuadTreeNode> right;
		std::shared_ptr<QuadTreeNode> bottom;

		std::shared_ptr<QuadTreeNode> parent;

		std::shared_ptr<QuadTreeNode> tl;
		std::shared_ptr<QuadTreeNode> tr;
		std::shared_ptr<QuadTreeNode> bl;
		std::shared_ptr<QuadTreeNode> br;	

		void getAllChildren(std::vector<std::shared_ptr<QuadTreeNode>>& children)
		{
			getChildren(this->tl, children);
			getChildren(this->tr, children);
			getChildren(this->bl, children);
			getChildren(this->br, children);
		}

		bool isLeaf()
		{
			return tl == nullptr && tr == nullptr && bl == nullptr && br == nullptr;
		}	

	private:

		void getChildren(const std::shared_ptr<QuadTreeNode>& node, std::vector<std::shared_ptr<QuadTreeNode>>& children)
		{
			if (node == nullptr)
			{
				return;
			}

			if (node->isLeaf())
			{
				children.push_back(node);
			}
			else
			{
				getChildren(node->tl, children);
				getChildren(node->tr, children);
				getChildren(node->bl, children);
				getChildren(node->br, children);
			}
		}
	};

	class QuadTree
	{
	public:

		std::vector<std::shared_ptr<QuadTreeNode>> nodes;
		std::shared_ptr<QuadTreeNode> root;
		Image8U3 image;
		int max_level;

		QuadTree(const Image8U3& image, int minBlockSize, float minStdDev) : minBlockSize(minBlockSize), minStdDev(minStdDev)
		{
			//convert to LAB space
			cv::cvtColor(image, this->image, cv::COLOR_BGR2Lab);


			mask = ImageUI::zeros(image.rows, image.cols);
			id = 0;
			root = std::make_unique<QuadTreeNode>(id, 0, 0, image.cols, image.rows, 0, nullptr, 0);
			max_level = 0;
			
			process(root);

			fixNeighbours(root);
		}

		void getNodeChildren(std::shared_ptr<QuadTreeNode> node, std::vector<std::shared_ptr<QuadTreeNode>>& children)
		{
			if (node == nullptr)
			{
				return;
			}

			if (node->isLeaf())
			{
				children.push_back(node);
			}
			else
			{
				node->getAllChildren(children);
			}
		}

		void getNodeChildren(std::shared_ptr<QuadTreeNode> node, std::shared_ptr<QuadTreeNode> neighbour, std::vector<std::shared_ptr<QuadTreeNode>>& children)
		{
			if (neighbour == nullptr)
			{
				return;
			}

			if (neighbour->isLeaf() && neighbour->level >= node->level)
			{
				children.push_back(neighbour);
			}
			else
			{
				neighbour->getAllChildren(children);
			}
		}

		void neighboursRight(std::shared_ptr<QuadTreeNode> node, std::vector<std::shared_ptr<QuadTreeNode>>& neighbours)
		{
			if (node->right != nullptr)
			{
				getLeftLeaves(node->right, neighbours);
			}
		}

		void neighboursLeft(std::shared_ptr<QuadTreeNode> node, std::vector<std::shared_ptr<QuadTreeNode>>& neighbours)
		{
			if (node->left != nullptr)
			{
				getRightLeaves(node->left, neighbours);
			}
		}

		void neighboursTop(std::shared_ptr<QuadTreeNode> node, std::vector<std::shared_ptr<QuadTreeNode>>& neighbours)
		{
			if (node->top != nullptr)
			{
				getBottomLeaves(node->top, neighbours);
			}
		}

		void neighboursBottom(std::shared_ptr<QuadTreeNode> node, std::vector<std::shared_ptr<QuadTreeNode>>& neighbours)
		{
			if (node->bottom != nullptr)
			{
				getTopLeaves(node->bottom, neighbours);
			}
		}

		void neighboursBottomRight(std::shared_ptr<QuadTreeNode> node, std::vector<std::shared_ptr<QuadTreeNode>>& neighbours)
		{
			if (node->bottom != nullptr && node->bottom->right != nullptr)
			{
				getLeftLeaves(node->bottom->right, neighbours);
				getTopLeaves(node->bottom->right, neighbours);
			}
		}

		void neighboursBottomLeft(std::shared_ptr<QuadTreeNode> node, std::vector<std::shared_ptr<QuadTreeNode>>& neighbours)
		{
			if (node->bottom != nullptr && node->bottom->left != nullptr)
			{
				getRightLeaves(node->bottom->left, neighbours);
				getTopLeaves(node->bottom->left, neighbours);
			}
		}

		void neighboursTopRight(std::shared_ptr<QuadTreeNode> node, std::vector<std::shared_ptr<QuadTreeNode>>& neighbours)
		{
			if (node->top != nullptr && node->top->right != nullptr)
			{
				getLeftLeaves(node->top->right, neighbours);
				getBottomLeaves(node->top->right, neighbours);
			}
		}

		void neighboursTopLeft(std::shared_ptr<QuadTreeNode> node, std::vector<std::shared_ptr<QuadTreeNode>>& neighbours)
		{
			if (node->top != nullptr && node->top->left != nullptr)
			{
				getRightLeaves(node->top->left, neighbours);
				getBottomLeaves(node->top->left, neighbours);
			}
		}

		void getAllChildren(std::shared_ptr<QuadTreeNode> node, std::vector<std::shared_ptr<QuadTreeNode>>& children)
		{
			getChildren(node->tl, children);
			getChildren(node->tr, children);
			getChildren(node->bl, children);
			getChildren(node->br, children);
		}

		int getNeighbourBlock(const ImageRef& pixel)
		{
			if (pixel.x >= 0 && pixel.x < mask.width() && pixel.y >= 0 && pixel.y < mask.height())
			{
				return mask(pixel);
			}

			return -1;
		}

		ImageUI mask;		

	private:

		std::list<std::shared_ptr<QuadTreeNode>> nodesTmp;

		// Fixes the pointers of all the nodes
		void fixNeighbours(std::shared_ptr<QuadTreeNode> node)
		{
			if (!node->isLeaf())
			{
				node->tl->left = node->left != nullptr ? (node->left->tr != nullptr ? node->left->tr : node->left) : nullptr;
				node->tl->top = node->top != nullptr ? (node->top->bl != nullptr ? node->top->bl : node->top) : nullptr;

				node->tr->right = node->right != nullptr ? (node->right->tl != nullptr ? node->right->tl : node->right) : nullptr;
				node->tr->top = node->top != nullptr ? (node->top->br != nullptr ? node->top->br : node->top) : nullptr;

				node->bl->left = node->left != nullptr ? (node->left->br != nullptr ? node->left->br : node->left) : nullptr;
				node->bl->bottom = node->bottom != nullptr ? (node->bottom->tl != nullptr ? node->bottom->tl : node->bottom) : nullptr;

				node->br->right = node->right != nullptr ? (node->right->bl != nullptr ? node->right->bl : node->right) : nullptr;
				node->br->bottom = node->bottom != nullptr ? (node->bottom->tr != nullptr ? node->bottom->tr : node->bottom) : nullptr;

				fixNeighbours(node->tl);
				fixNeighbours(node->tr);
				fixNeighbours(node->bl);
				fixNeighbours(node->br);
			}
		}

		void splitQaud(std::shared_ptr<QuadTreeNode> node, std::shared_ptr<QuadTreeNode>& tl, std::shared_ptr<QuadTreeNode>& tr, std::shared_ptr<QuadTreeNode>& bl, std::shared_ptr<QuadTreeNode>& br)
		{
			tl = std::make_shared<QuadTreeNode>(++id, node->x, node->y, node->width / 2, node->height / 2, node->level + 1, node, 0);
			tr = std::make_shared<QuadTreeNode>(++id, node->x + node->width / 2, node->y, node->width - node->width / 2, node->height / 2, node->level + 1, node, 1);
			bl = std::make_shared<QuadTreeNode>(++id, node->x, node->y + node->height / 2, node->width / 2, node->height - node->height / 2, node->level + 1, node, 1);
			br = std::make_shared<QuadTreeNode>(++id, node->x + node->width / 2, node->y + node->height / 2, node->width - node->width / 2, node->height - node->height / 2, node->level + 1, node, 0);

			// children
			node->tl = tl;
			node->tr = tr;
			node->bl = bl;
			node->br = br;

			tl->right = tr;
			tl->bottom = bl;

			tr->left = tl;
			tr->bottom = br;

			bl->top = tl;
			bl->right = br;

			br->left = bl;
			br->top = tr;
		}

		void process(std::shared_ptr<QuadTreeNode> node)
		{
			
			if (node->width > 0 && node->height > 0)
			{
				cv::Mat crop = image(cv::Rect(node->x, node->y, node->width, node->height));

				cv::Scalar mean;
				cv::Scalar stddev;
				cv::meanStdDev(crop, mean, stddev);

				//std::cout << "stddev: " << stddev << std::endl;	

				// s as a mean of all three (actually four) channels
				float s = cv::mean(stddev).val[0];
				//float s = stddev.val[0];
				//float s_ = (stddev.val[1]+stddev.val[2])/2;

				if (s >= minStdDev && std::min(node->width, node->height) > minBlockSize)
				{
					std::shared_ptr<QuadTreeNode> tl;
					std::shared_ptr<QuadTreeNode> tr;
					std::shared_ptr<QuadTreeNode> bl;
					std::shared_ptr<QuadTreeNode> br;

					splitQaud(node, tl, tr, bl, br);
					//max_level = max_level +1;
					
					process(tl);
					process(tr);
					process(bl);
					process(br);
				}
				else
				{
					mask(cv::Rect(node->x, node->y, node->width, node->height)) = nodes.size();
					node->avgColor = cv::Vec3b(mean.val[0], mean.val[1], mean.val[2]);
					//node->abColor = cv::Vec3b(0, mean.val[1], mean.val[2]);
					node->id = nodes.size();
					nodes.push_back(node);
					max_level = std::max(max_level, node->level);
				}
			}
		}

		void getChildren(const std::shared_ptr<QuadTreeNode>& node, std::vector<std::shared_ptr<QuadTreeNode>>& children)
		{
			if (node == nullptr)
			{
				return;
			}

			if (node->isLeaf())
			{
				children.push_back(node);
			}
			else
			{
				getChildren(node->tl, children);
				getChildren(node->tr, children);
				getChildren(node->bl, children);
				getChildren(node->br, children);
			}
		}

		void getLeftLeaves(const std::shared_ptr<QuadTreeNode>& node, std::vector<std::shared_ptr<QuadTreeNode>>& leaves)
		{
			if (node == nullptr)
			{
				return;
			}

			if (node->isLeaf())
			{
				leaves.push_back(node);
			}
			else
			{
				getLeftLeaves(node->tl, leaves);
				getLeftLeaves(node->bl, leaves);
			}
		}

		void getRightLeaves(const std::shared_ptr<QuadTreeNode>& node, std::vector<std::shared_ptr<QuadTreeNode>>& leaves)
		{
			if (node == nullptr)
			{
				return;
			}

			if (node->isLeaf())
			{
				leaves.push_back(node);
			}
			else
			{
				getRightLeaves(node->tr, leaves);
				getRightLeaves(node->br, leaves);
			}
		}

		void getTopLeaves(const std::shared_ptr<QuadTreeNode>& node, std::vector<std::shared_ptr<QuadTreeNode>>& leaves)
		{
			if (node == nullptr)
			{
				return;
			}

			if (node->isLeaf())
			{
				leaves.push_back(node);
			}
			else
			{
				getTopLeaves(node->tl, leaves);
				getTopLeaves(node->tr, leaves);
			}
		}

		void getBottomLeaves(const std::shared_ptr<QuadTreeNode>& node, std::vector<std::shared_ptr<QuadTreeNode>>& leaves)
		{
			if (node == nullptr)
			{
				return;
			}

			if (node->isLeaf())
			{
				leaves.push_back(node);
			}
			else
			{
				getBottomLeaves(node->bl, leaves);
				getBottomLeaves(node->br, leaves); 
			}
		}

		

		int id;
		int minBlockSize;
		float minStdDev;		
	};
}

#endif