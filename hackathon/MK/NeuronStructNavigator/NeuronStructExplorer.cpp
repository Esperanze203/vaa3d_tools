#include <iostream>
#include <algorithm>
#include <cmath>

#include "basic_4dimage.h"
#include "basic_landmark.h"
#include "neuron_format_converter.h"

#include "NeuronStructExplorer.h"

using namespace std;

profiledTree::profiledTree(const NeuronTree& inputTree)
{
	this->tree = inputTree;
	for (QList<NeuronSWC>::iterator it = this->tree.listNeuron.begin(); it != this->tree.listNeuron.end(); ++it)
	{
		string xLabel = to_string(int(it->x / 40));
		string yLabel = to_string(int(it->y / 40));
		string zLabel = to_string(int(it->z / (40 / zRATIO)));
		string keyLabel = xLabel + "_" + yLabel + "_" + zLabel;
		if (this->nodeTileMap.find(keyLabel) != this->nodeTileMap.end()) this->nodeTileMap[keyLabel].push_back(it->n);
		else
		{
			vector<int> newSet;
			newSet.push_back(it->n);
			this->nodeTileMap.insert(pair<string, vector<int>>(keyLabel, newSet));
		}
	}
	NeuronStructUtil::node2loc_node2childLocMap(this->tree.listNeuron, this->node2LocMap, this->node2childLocMap);
	this->segs = NeuronStructExplorer::findSegs(this->tree.listNeuron, this->node2childLocMap);
	cout << "segs num: " << this->segs.size() << endl;
	vector<segUnit> allSegs;
	for (map<int, segUnit>::iterator it = this->segs.begin(); it != this->segs.end(); ++it)
	{	
		//if (it->second.tails.size() > 1) cout << " branching seg: " << it->first << endl;
		allSegs.push_back(it->second);
	}
	this->segHeadMap = NeuronStructExplorer::segTileMap(allSegs);
	this->segTailMap = NeuronStructExplorer::segTileMap(allSegs, false);
}

NeuronStructExplorer::NeuronStructExplorer(QString inputFileName)
{
	QStringList fileNameParse1 = inputFileName.split(".");
	if (fileNameParse1.back() == "swc")
	{
		NeuronTree inputSWC = readSWC_file(inputFileName);
		QStringList fileNameParse2 = fileNameParse1.at(0).split("\\");
		string treeName = fileNameParse2.back().toStdString();
		this->treeEntry(inputSWC, treeName);
		
		this->singleTree = inputSWC;
		this->singleTreePtr = &(this->singleTree);
	}
}

void NeuronStructExplorer::treeEntry(const NeuronTree& inputTree, string treeName)
{
	if (this->treeDataBase.find(treeName) == this->treeDataBase.end())
	{
		profiledTree registeredTree(inputTree);
		this->treeDataBase.insert(pair<string, profiledTree>(treeName, registeredTree));
	}
	else
	{
		cerr << "This tree name has already existed. The tree will not be registered for further operations." << endl;
		return;
	}
}

NeuronTree NeuronStructExplorer::SWC2MSTtree(NeuronTree const& inputTreePtr)
{
	NeuronTree MSTtrees;
	undirectedGraph graph(inputTreePtr.listNeuron.size());
	for (int i = 0; i < inputTreePtr.listNeuron.size(); ++i)
	{
			
		float x1, y1, z1;
		x1 = inputTreePtr.listNeuron.at(i).x;
		y1 = inputTreePtr.listNeuron.at(i).y;
		z1 = inputTreePtr.listNeuron.at(i).z;
		for (int j = 0; j < inputTreePtr.listNeuron.size(); ++j)
		{
			float x2, y2, z2;
			x2 = inputTreePtr.listNeuron.at(j).x;
			y2 = inputTreePtr.listNeuron.at(j).y;
			z2 = inputTreePtr.listNeuron.at(j).z;

			double Vedge = sqrt(double(x1 - x2) * double(x1 - x2) + double(y1 - y2) * double(y1 - y2) + zRATIO * zRATIO * double(z1 - z2) * double(z1 - z2));
			pair<undirectedGraph::edge_descriptor, bool> edgeQuery = boost::edge(i, j, graph);
			if (!edgeQuery.second && i != j) boost::add_edge(i, j, lastVoted(i, weights(Vedge)), graph);
		}
	}

	vector <boost::graph_traits<undirectedGraph>::vertex_descriptor > p(num_vertices(graph));
	boost::prim_minimum_spanning_tree(graph, &p[0]);
	NeuronTree MSTtree;
	QList<NeuronSWC> listNeuron;
	QHash<int, int>  hashNeuron;
	listNeuron.clear();
	hashNeuron.clear();
		
	for (size_t ii = 0; ii != p.size(); ++ii)
	{
		int pn;
		if (p[ii] == ii) pn = -1;
		else pn = p[ii] + 1;

		NeuronSWC S;
		S.n = ii + 1;
		S.type = 7;
		S.x = inputTreePtr.listNeuron.at(ii).x;
		S.y = inputTreePtr.listNeuron.at(ii).y;
		S.z = inputTreePtr.listNeuron.at(ii).z;
		S.r = 1;
		S.pn = pn;
		listNeuron.append(S);
		hashNeuron.insert(S.n, listNeuron.size() - 1);
	}
	MSTtree.listNeuron = listNeuron;
	MSTtree.hashNeuron = hashNeuron;

	return MSTtree;
}

map<int, segUnit> NeuronStructExplorer::findSegs(const QList<NeuronSWC>& inputNodeList, map<int, vector<size_t>>& node2childLocMap)
{
	// -- This method profiles all segments in a given input tree.

	map<int, segUnit> segs;
	int segCount = 0;
	for (QList<NeuronSWC>::const_iterator nodeIt = inputNodeList.begin(); nodeIt != inputNodeList.end(); ++nodeIt)
	{
		if (nodeIt->parent == -1)
		{
			// ------------ Identify segments ------------
			++segCount;
			//cout << " processing segment " << segCount << "..  head node ID: " << nodeIt->n << endl << "  ";
			segUnit newSeg;
			newSeg.segID = segCount;
			newSeg.head = nodeIt->n;
			newSeg.nodes.push_back(*nodeIt);
			vector<size_t> childLocs = node2childLocMap[nodeIt->n];
			vector<size_t> grandChildLocs;
			vector<size_t> thisGrandChildLocs;
			bool childNum = true;
			while (childNum)
			{
				grandChildLocs.clear();
				for (vector<size_t>::iterator childIt = childLocs.begin(); childIt != childLocs.end(); ++childIt)
				{
					//cout << inputNodeList[*childIt].n << " ";
					thisGrandChildLocs.clear();
					for (QList<NeuronSWC>::iterator checkIt = newSeg.nodes.begin(); checkIt != newSeg.nodes.end(); ++checkIt)
					{
						if (checkIt->n == inputNodeList[*childIt].n) goto LOOP;
					}

					newSeg.nodes.push_back(inputNodeList[*childIt]);
					thisGrandChildLocs = node2childLocMap[inputNodeList[*childIt].n];
					if (thisGrandChildLocs.size() == 0) newSeg.tails.push_back(inputNodeList[*childIt].n);

					grandChildLocs.insert(grandChildLocs.end(), thisGrandChildLocs.begin(), thisGrandChildLocs.end());

				LOOP:
					continue;
				}
				if (grandChildLocs.size() == 0) childNum = false;

				childLocs = grandChildLocs;
			}
			//cout << endl; 
			// --------- END of [Identify segments] ---------

			// ------------- Identify node/child location in the segment -------------
			for (QList<NeuronSWC>::iterator it = newSeg.nodes.begin(); it != newSeg.nodes.end(); ++it) 
				newSeg.seg_nodeLocMap.insert(pair<int, size_t>(it->n, size_t(it - newSeg.nodes.begin())));

			vector<size_t> childSegLocs;
			newSeg.seg_childLocMap.insert(pair<int, vector<size_t>>(newSeg.nodes.begin()->n, childSegLocs));
			for (QList<NeuronSWC>::iterator it = newSeg.nodes.begin(); it != newSeg.nodes.end(); ++it)
			{
				if (it->parent == -1) continue;

				size_t paLoc = newSeg.seg_nodeLocMap[it->parent];
				if (newSeg.seg_childLocMap.find(newSeg.nodes[paLoc].n) != newSeg.seg_childLocMap.end()) newSeg.seg_childLocMap[newSeg.nodes[paLoc].n].push_back(newSeg.seg_nodeLocMap[it->n]);
				else
				{
					vector<size_t> childSet;
					childSet.push_back(newSeg.seg_nodeLocMap[it->n]);
					newSeg.seg_childLocMap.insert(pair<int, vector<size_t>>(newSeg.nodes[newSeg.seg_nodeLocMap[it->parent]].n, childSet));
				}
			}
			segs.insert(pair<int, segUnit>(newSeg.segID, newSeg));
			// --------- END of [Identify node/child location in the segment] ---------
		}
	}

	return segs;
}

map<string, vector<int>> NeuronStructExplorer::segTileMap(const vector<segUnit>& inputSegs, bool head, float xyLength, float xy2zRatio)
{
	if (head)
	{
		map<string, vector<int>> outputSegTileMap;
		for (vector<segUnit>::const_iterator it = inputSegs.begin(); it != inputSegs.end(); ++it)
		{
			string xLabel = to_string(int(it->nodes.begin()->x / xyLength));
			string yLabel = to_string(int(it->nodes.begin()->y / xyLength));
			string zLabel = to_string(int(it->nodes.begin()->z / (xyLength * xy2zRatio)));
			string keyLabel = xLabel + "_" + yLabel + "_" + zLabel;
			if (outputSegTileMap.find(keyLabel) != outputSegTileMap.end()) outputSegTileMap[keyLabel].push_back(it->segID);
			else
			{
				vector<int> newSet;
				newSet.push_back(it->segID);
				outputSegTileMap.insert(pair<string, vector<int>>(keyLabel, newSet));
			}
		}
		return outputSegTileMap;
	}
	else
	{
		map<string, vector<int>> outputSegTileMap;
		for (vector<segUnit>::const_iterator it = inputSegs.begin(); it != inputSegs.end(); ++it)
		{
			for (QList<NeuronSWC>::const_iterator it2 = it->nodes.begin(); it2 != it->nodes.end(); ++it2)
			{
				if (it->seg_childLocMap.find(it2->n) == it->seg_childLocMap.end())
				{
					string xLabel = to_string(int(it2->x / xyLength));
					string yLabel = to_string(int(it2->y / xyLength));
					string zLabel = to_string(int(it2->z / (xyLength * xy2zRatio)));
					string keyLabel = xLabel + "_" + yLabel + "_" + zLabel;
					if (outputSegTileMap.find(keyLabel) != outputSegTileMap.end()) outputSegTileMap[keyLabel].push_back(it->segID);
					else
					{
						vector<int> newSet;
						newSet.push_back(it->segID);
						outputSegTileMap.insert(pair<string, vector<int>>(keyLabel, newSet));
					}
				}
			}
		}
		return outputSegTileMap;
	}
}

NeuronTree NeuronStructExplorer::MSTbranchBreak(const profiledTree& inputProfiledTree, bool spikeRemove)
{
	// -- This method breaks all branching points of a MST-connected tree.
	// -- When the value of spikeRemove is true, it eliminates spikes on a main route without breaking it. The default value is true. 

	NeuronTree outputTree = inputProfiledTree.tree;
	
	vector<size_t> spikeLocs;
	if (spikeRemove)
	{
		for (QList<NeuronSWC>::iterator it = outputTree.listNeuron.begin(); it != outputTree.listNeuron.end(); ++it)
		{
			vector<size_t> childLocs = inputProfiledTree.node2childLocMap.at(it->n);
			if (childLocs.size() >= 2)
			{
				int nodeRemoveCount = 0;
				for (vector<size_t>::iterator locIt = childLocs.begin(); locIt != childLocs.end(); ++locIt)
				{
					if (inputProfiledTree.node2childLocMap.at(outputTree.listNeuron[*locIt].n).size() == 0)
					{
						double spikeDist = sqrt((outputTree.listNeuron[*locIt].x - it->x) * (outputTree.listNeuron[*locIt].x - it->x) +
												(outputTree.listNeuron[*locIt].y - it->y) * (outputTree.listNeuron[*locIt].y - it->y) +
												(outputTree.listNeuron[*locIt].z - it->z) * (outputTree.listNeuron[*locIt].z - it->z) * zRATIO * zRATIO);
						if (spikeDist <= 10) // Take out splikes.
						{
							spikeLocs.push_back(*locIt);
							++nodeRemoveCount;
						}
					}
				}

				if (nodeRemoveCount == childLocs.size() - 1) continue; // If there is only 1 child left after taking out all spikes, then this node is supposed to be on the main route.
				else
				{
					for (vector<size_t>::iterator locCheckIt = childLocs.begin(); locCheckIt != childLocs.end(); ++locCheckIt)
					{
						if (find(spikeLocs.begin(), spikeLocs.end(), *locCheckIt) == spikeLocs.end())
							outputTree.listNeuron[*locCheckIt].parent = -1;
					}
				}
			}
		}
	}
	else
	{
		for (QList<NeuronSWC>::iterator it = outputTree.listNeuron.begin(); it != outputTree.listNeuron.end(); ++it)
		{
			if (inputProfiledTree.node2childLocMap.at(it->n).size() >= 2)
			{
				for (vector<size_t>::const_iterator locIt = inputProfiledTree.node2childLocMap.at(it->n).begin(); locIt != inputProfiledTree.node2childLocMap.at(it->n).end(); ++locIt)
					outputTree.listNeuron[*locIt].parent = -1;
			}
		}
	}
	
	if (spikeRemove) // Erase spike nodes from outputTree.listNeuron.
	{
		sort(spikeLocs.rbegin(), spikeLocs.rend());
		for (vector<size_t>::iterator delIt = spikeLocs.begin(); delIt != spikeLocs.end(); ++delIt) 
			outputTree.listNeuron.erase(outputTree.listNeuron.begin() + ptrdiff_t(*delIt));
	}

	return outputTree;
}

double NeuronStructExplorer::segPointingCompare(const segUnit& elongSeg, const segUnit& connSeg, connectOrientation connOrt)
{
	if (elongSeg.tails.size() > 1 || connSeg.tails.size() > 1)
	{
		cerr << "Invalid input: Currently segment elongation only allows to happen between 2 non-branching segments." << endl;
		return -1;
	}

	NeuronSWC elongHeadNode = elongSeg.nodes[elongSeg.seg_nodeLocMap.at(elongSeg.head)];
	NeuronSWC elongTailNode;
	if (elongSeg.tails.size() == 0)
	{
		//cerr << "Elongating segment only has head. Do nothinig and return.";
		return -1;
	}
	else elongTailNode = elongSeg.nodes[elongSeg.seg_nodeLocMap.at(*elongSeg.tails.begin())];
	
	NeuronSWC connHeadNode = connSeg.nodes[connSeg.seg_nodeLocMap.at(connSeg.head)];
	NeuronSWC connTailNode;
	if (connSeg.tails.size() == 0)
	{
		if (connOrt == head_tail || connOrt == tail_tail)
		{
			//cerr << "Connecting segment only has head. Do nothing and return";
			return -1;
		}
	}
	else connTailNode = connSeg.nodes[connSeg.seg_nodeLocMap.at(*connSeg.tails.begin())];

	vector<float> elongHeadLoc;
	elongHeadLoc.push_back(elongHeadNode.x);
	elongHeadLoc.push_back(elongHeadNode.y);
	elongHeadLoc.push_back(elongHeadNode.z * zRATIO);

	vector<float> elongTailLoc;
	elongTailLoc.push_back(elongTailNode.x);
	elongTailLoc.push_back(elongTailNode.y);
	elongTailLoc.push_back(elongTailNode.z * zRATIO);

	vector<float> connHeadLoc;
	connHeadLoc.push_back(connHeadNode.x);
	connHeadLoc.push_back(connHeadNode.y);
	connHeadLoc.push_back(connHeadNode.z * zRATIO);

	vector<float> connTailLoc;
	connTailLoc.push_back(connTailNode.x);
	connTailLoc.push_back(connTailNode.y);
	connTailLoc.push_back(connTailNode.z * zRATIO);

	vector<float> elongDispUnitVec;
	vector<float> connDispUnitVec;
	if (connOrt == head_head)
	{
		elongDispUnitVec = this->getDispUnitVector(elongHeadLoc, elongTailLoc);
		connDispUnitVec = this->getDispUnitVector(connTailLoc, connHeadLoc);

		double radAngle = this->getRadAngle(elongDispUnitVec, connDispUnitVec);
		return radAngle;
	}
	else if (connOrt == head_tail)
	{
		elongDispUnitVec = this->getDispUnitVector(elongHeadLoc, elongTailLoc);
		connDispUnitVec = this->getDispUnitVector(connHeadLoc, connTailLoc);

		double radAngle = this->getRadAngle(elongDispUnitVec, connDispUnitVec);
		return radAngle;
	}
	else if (connOrt == tail_head)
	{
		elongDispUnitVec = this->getDispUnitVector(elongHeadLoc, elongTailLoc);
		connDispUnitVec = this->getDispUnitVector(connHeadLoc, connTailLoc);

		double radAngle = this->getRadAngle(elongDispUnitVec, connDispUnitVec);
		return radAngle;
	}
	else if (connOrt == tail_tail)
	{
		elongDispUnitVec = this->getDispUnitVector(elongTailLoc, elongHeadLoc);
		connDispUnitVec = this->getDispUnitVector(connHeadLoc, connTailLoc);

		double radAngle = this->getRadAngle(elongDispUnitVec, connDispUnitVec);
		return radAngle;
	}
}

double NeuronStructExplorer::segTurningAngle(const segUnit& elongSeg, const segUnit& connSeg, connectOrientation connOrt)
{
	if (elongSeg.tails.size() > 1 || connSeg.tails.size() > 1)
	{
		cerr << "Invalid input: Currently segment elongation only allows to happen between 2 non-branching segments." << endl;
		return -1;
	}

	NeuronSWC elongHeadNode = elongSeg.nodes[elongSeg.seg_nodeLocMap.at(elongSeg.head)];
	NeuronSWC elongTailNode;
	if (elongSeg.tails.size() == 0)
	{
		//cerr << "Elongating segment only has head. Do nothinig and return.";
		return -1;
	}
	else elongTailNode = elongSeg.nodes[elongSeg.seg_nodeLocMap.at(*elongSeg.tails.begin())];

	NeuronSWC connHeadNode = connSeg.nodes[connSeg.seg_nodeLocMap.at(connSeg.head)];
	NeuronSWC connTailNode;
	if (connSeg.tails.size() == 0)
	{
		if (connOrt == head_tail || connOrt == tail_tail)
		{
			//cerr << "Connecting segment only has head. Do nothing and return";
			return -1;
		}
	}
	else connTailNode = connSeg.nodes[connSeg.seg_nodeLocMap.at(*connSeg.tails.begin())];

	vector<float> elongHeadLoc;
	elongHeadLoc.push_back(elongHeadNode.x);
	elongHeadLoc.push_back(elongHeadNode.y);
	elongHeadLoc.push_back(elongHeadNode.z * zRATIO);

	vector<float> elongTailLoc;
	elongTailLoc.push_back(elongTailNode.x);
	elongTailLoc.push_back(elongTailNode.y);
	elongTailLoc.push_back(elongTailNode.z * zRATIO);

	vector<float> elongDispUnitVec;
	vector<float> connPointUnitVec;
	if (connOrt == head_head)
	{
		elongDispUnitVec = this->getDispUnitVector(elongHeadLoc, elongTailLoc);
		vector<float> connPointLoc;
		connPointLoc.push_back(connHeadNode.x);
		connPointLoc.push_back(connHeadNode.y);
		connPointLoc.push_back(connHeadNode.z * zRATIO);
		connPointUnitVec = this->getDispUnitVector(connPointLoc, elongHeadLoc);

		double radAngle = this->getRadAngle(elongDispUnitVec, connPointUnitVec);
		return radAngle;
	}
	else if (connOrt == head_tail)
	{
		elongDispUnitVec = this->getDispUnitVector(elongHeadLoc, elongTailLoc);
		vector<float> connPointLoc;
		connPointLoc.push_back(connTailNode.x);
		connPointLoc.push_back(connTailNode.y);
		connPointLoc.push_back(connTailNode.z * zRATIO);
		connPointUnitVec = this->getDispUnitVector(connPointLoc, elongHeadLoc);

		double radAngle = this->getRadAngle(elongDispUnitVec, connPointUnitVec);
		return radAngle;
	}
	else if (connOrt == tail_head)
	{
		elongDispUnitVec = this->getDispUnitVector(elongTailLoc, elongHeadLoc);
		vector<float> connPointLoc;
		connPointLoc.push_back(connHeadNode.x);
		connPointLoc.push_back(connHeadNode.y);
		connPointLoc.push_back(connHeadNode.z * zRATIO);
		connPointUnitVec = this->getDispUnitVector(connPointLoc, elongTailLoc);

		double radAngle = this->getRadAngle(elongDispUnitVec, connPointUnitVec);
		return radAngle;
	}
	else if (connOrt == tail_tail)
	{
		elongDispUnitVec = this->getDispUnitVector(elongTailLoc, elongHeadLoc);
		vector<float> connPointLoc;
		connPointLoc.push_back(connTailNode.x);
		connPointLoc.push_back(connTailNode.y);
		connPointLoc.push_back(connTailNode.z * zRATIO);
		connPointUnitVec = this->getDispUnitVector(connPointLoc, elongTailLoc);

		double radAngle = this->getRadAngle(elongDispUnitVec, connPointUnitVec);
		return radAngle;
	}
}

segUnit NeuronStructExplorer::segUnitConnect(const segUnit& segUnit1, const segUnit& segUnit2, connectOrientation connOrt)
{
	if (segUnit1.tails.size() > 1 || segUnit2.tails.size() > 1)
		throw invalid_argument("Currently forked segment connection is not supported. Do nothing and return");

	segUnit newSeg;
	QList<NeuronSWC> newSegNodes;
	QList<NeuronSWC> endEditedNodes;

	switch (connOrt)
	{
	case head_tail:
	{
		int connTailID = segUnit2.nodes.at(segUnit2.seg_nodeLocMap.at(*segUnit2.tails.begin())).n;
		endEditedNodes = segUnit1.nodes;
		endEditedNodes.begin()->parent = connTailID;
		newSegNodes.append(segUnit2.nodes);
		newSegNodes.append(endEditedNodes);
		newSeg.nodes = newSegNodes;
		break;
	}
	case tail_head:
	{
		int connTailID = segUnit1.nodes.at(segUnit1.seg_nodeLocMap.at(*segUnit1.tails.begin())).n;
		endEditedNodes = segUnit2.nodes;
		endEditedNodes.begin()->parent = connTailID;
		newSegNodes.append(segUnit1.nodes);
		newSegNodes.append(endEditedNodes);
		newSeg.nodes = newSegNodes;
		break;
	}
	case head_head:
	{
		int connTailID = segUnit2.head;
		for (map<int, vector<size_t>>::const_iterator it = segUnit2.seg_childLocMap.begin(); it != segUnit2.seg_childLocMap.end(); ++it)
		{
			NeuronSWC newNode = segUnit2.nodes.at(segUnit2.seg_nodeLocMap.at(it->first));
			if (it->second.size() == 0)
			{
				newNode.parent = -1;
				system("pause");
			}
			else newNode.parent = segUnit2.nodes.at(*(it->second.cbegin())).n;
			endEditedNodes.push_back(newNode);
		}
		newSegNodes.append(segUnit1.nodes);
		newSegNodes.begin()->parent = connTailID;
		newSegNodes.append(endEditedNodes);
		newSeg.nodes = newSegNodes;
		break;
	}
	case tail_tail:
	{
		int connTailID = *segUnit2.tails.cbegin();
		for (map<int, vector<size_t>>::const_iterator it = segUnit1.seg_childLocMap.begin(); it != segUnit1.seg_childLocMap.end(); ++it)
		{
			NeuronSWC newNode = segUnit1.nodes.at(segUnit1.seg_nodeLocMap.at(it->first));
			if (it->second.size() == 0) newNode.parent = connTailID;
			else newNode.parent = segUnit1.nodes.at(*(it->second.begin())).n;
			endEditedNodes.push_back(newNode);
		}
		newSegNodes.append(segUnit2.nodes);
		newSegNodes.append(endEditedNodes);
		newSeg.nodes = newSegNodes;
		break;
	}
	default:
		break;
	}
	newSeg.nodes = newSegNodes;

	return newSeg;
}

map<int, segUnit> NeuronStructExplorer::segRegionConn_angle(const vector<int>& currTileHeadSegIDs, const vector<int>& currTileTailSegIDs, profiledTree& currProfiledTree, bool length)
{
	set<int> connectedSegs;
	map<int, int> elongConnMap;
	map<string, double> segAngleMap;
	map<int, segUnit> newSegs;

	//__________________________________________________________________________________________________________________________
	
	cout << "------- head-tail:" << endl;
	for (vector<int>::const_iterator headIt = currTileHeadSegIDs.begin(); headIt != currTileHeadSegIDs.end(); ++headIt)
	{
		for (vector<int>::const_iterator tailIt = currTileTailSegIDs.begin(); tailIt != currTileTailSegIDs.end(); ++tailIt)
		{
			if (*headIt == *tailIt) continue;
			else
			{
				if (currProfiledTree.segs.at(*headIt).to_be_deted || currProfiledTree.segs.at(*tailIt).to_be_deted) continue;

				double pointingRadAngle = this->segPointingCompare(currProfiledTree.segs.at(*headIt), currProfiledTree.segs.at(*tailIt), head_tail);
				double turningRadAngle = this->segTurningAngle(currProfiledTree.segs.at(*headIt), currProfiledTree.segs.at(*tailIt), head_tail);
				if (pointingRadAngle == -1 || turningRadAngle == -1) continue;

				if (pointingRadAngle < 0.25 && turningRadAngle < 0.25)
				{
					cout << *headIt << "_" << *tailIt << "->(" << pointingRadAngle << "," << turningRadAngle << ") ";
					string segAngleKey = to_string(*headIt) + "_" + to_string(*tailIt);
					segAngleMap.insert(pair<string, double>(segAngleKey, (pointingRadAngle + turningRadAngle)));
				}
			}
		}
	}
	this->tileSegConnOrganizer_angle(segAngleMap, connectedSegs, elongConnMap);
	for (map<int, int>::iterator it = elongConnMap.begin(); it != elongConnMap.end(); ++it)
	{
		currProfiledTree.segs[it->first].to_be_deted = true;
		currProfiledTree.segs[it->second].to_be_deted = true;
		segUnit newSeg = this->segUnitConnect(currProfiledTree.segs[it->first], currProfiledTree.segs[it->second], head_tail);
		newSeg.segID = currProfiledTree.segs.size() + 1;
		currProfiledTree.segs.insert(pair<int, segUnit>(newSeg.segID, newSeg));
		newSegs.insert(pair<int, segUnit>(newSeg.segID, newSeg));
	}
	segAngleMap.clear();
	elongConnMap.clear();
	cout << endl;
	//__________________________________________________________________________________________________________________________

	cout << "------- tail-head:" << endl;
	for (vector<int>::const_iterator tailIt = currTileHeadSegIDs.begin(); tailIt != currTileHeadSegIDs.end(); ++tailIt)
	{
		for (vector<int>::const_iterator headIt = currTileTailSegIDs.begin(); headIt != currTileTailSegIDs.end(); ++headIt)
		{
			if (*headIt == *tailIt) continue;
			else
			{
				if (currProfiledTree.segs.at(*tailIt).to_be_deted || currProfiledTree.segs.at(*headIt).to_be_deted) continue;

				double pointingRadAngle = this->segPointingCompare(currProfiledTree.segs.at(*tailIt), currProfiledTree.segs.at(*headIt), tail_head);
				double turningRadAngle = this->segTurningAngle(currProfiledTree.segs.at(*tailIt), currProfiledTree.segs.at(*headIt), tail_head);
				if (pointingRadAngle == -1 || turningRadAngle == -1) continue;

				if (pointingRadAngle < 0.25 && turningRadAngle < 0.25)
				{
					cout << *tailIt << "_" << *headIt << "->(" << pointingRadAngle << "," << turningRadAngle << ") ";
					string segAngleKey = to_string(*tailIt) + "_" + to_string(*headIt);
					segAngleMap.insert(pair<string, double>(segAngleKey, (pointingRadAngle + turningRadAngle)));
				}
			}
		}
	}
	this->tileSegConnOrganizer_angle(segAngleMap, connectedSegs, elongConnMap);
	for (map<int, int>::iterator it = elongConnMap.begin(); it != elongConnMap.end(); ++it)
	{
		currProfiledTree.segs[it->first].to_be_deted = true;
		currProfiledTree.segs[it->second].to_be_deted = true;
		segUnit newSeg = this->segUnitConnect(currProfiledTree.segs[it->first], currProfiledTree.segs[it->second], tail_head);
		newSeg.segID = currProfiledTree.segs.size() + 1;
		currProfiledTree.segs.insert(pair<int, segUnit>(newSeg.segID, newSeg));
		newSegs.insert(pair<int, segUnit>(newSeg.segID, newSeg));
	}
	segAngleMap.clear();
	elongConnMap.clear();
	cout << endl;
	//__________________________________________________________________________________________________________________________

	cout << "------- head-head:" << endl;
	for (vector<int>::const_iterator headIt1 = currTileHeadSegIDs.begin(); headIt1 != currTileHeadSegIDs.end(); ++headIt1)
	{
		for (vector<int>::const_iterator headIt2 = currTileHeadSegIDs.begin(); headIt2 != currTileHeadSegIDs.end(); ++headIt2)
		{
			if (*headIt1 == *headIt2) continue;
			else
			{
				if (currProfiledTree.segs.at(*headIt1).to_be_deted || currProfiledTree.segs.at(*headIt2).to_be_deted) continue;

				double pointingRadAngle = this->segPointingCompare(currProfiledTree.segs.at(*headIt1), currProfiledTree.segs.at(*headIt2), head_head);
				double turningRadAngle = this->segTurningAngle(currProfiledTree.segs.at(*headIt1), currProfiledTree.segs.at(*headIt2), head_head);
				if (pointingRadAngle == -1 || turningRadAngle == -1) continue;

				if (pointingRadAngle < 0.25 && turningRadAngle < 0.25)
				{
					cout << *headIt1 << "_" << *headIt2 << "->(" << pointingRadAngle << "," << turningRadAngle << ") ";
					string segAngleKey = to_string(*headIt1) + "_" + to_string(*headIt2);
					segAngleMap.insert(pair<string, double>(segAngleKey, (pointingRadAngle + turningRadAngle)));
				}
			}
		}
	}
	this->tileSegConnOrganizer_angle(segAngleMap, connectedSegs, elongConnMap);
	for (map<int, int>::iterator it = elongConnMap.begin(); it != elongConnMap.end(); ++it)
	{
		currProfiledTree.segs[it->first].to_be_deted = true;
		currProfiledTree.segs[it->second].to_be_deted = true;
		segUnit newSeg = this->segUnitConnect(currProfiledTree.segs[it->first], currProfiledTree.segs[it->second], head_head);
		newSeg.segID = currProfiledTree.segs.size() + 1;
		currProfiledTree.segs.insert(pair<int, segUnit>(newSeg.segID, newSeg));
		newSegs.insert(pair<int, segUnit>(newSeg.segID, newSeg));
	}
	segAngleMap.clear();
	elongConnMap.clear();
	cout << endl;
	//__________________________________________________________________________________________________________________________

	cout << "------- tail-tail:" << endl;
	for (vector<int>::const_iterator tailIt1 = currTileHeadSegIDs.begin(); tailIt1 != currTileHeadSegIDs.end(); ++tailIt1)
	{
		for (vector<int>::const_iterator tailIt2 = currTileTailSegIDs.begin(); tailIt2 != currTileTailSegIDs.end(); ++tailIt2)
		{
			if (*tailIt1 == *tailIt2) continue;
			else
			{
				if (currProfiledTree.segs.at(*tailIt1).to_be_deted || currProfiledTree.segs.at(*tailIt2).to_be_deted) continue;

				double pointingRadAngle = this->segPointingCompare(currProfiledTree.segs.at(*tailIt1), currProfiledTree.segs.at(*tailIt2), tail_tail);
				double turningRadAngle = this->segTurningAngle(currProfiledTree.segs.at(*tailIt1), currProfiledTree.segs.at(*tailIt2), tail_tail);
				if (pointingRadAngle == -1 || turningRadAngle == -1) continue;

				if (pointingRadAngle < 0.25 && turningRadAngle < 0.25)
				{
					cout << *tailIt1 << "_" << *tailIt2 << "->(" << pointingRadAngle << "," << turningRadAngle << ") ";
					string segAngleKey = to_string(*tailIt1) + "_" + to_string(*tailIt2);
					segAngleMap.insert(pair<string, double>(segAngleKey, (pointingRadAngle + turningRadAngle)));
				}
			}
		}
	}
	this->tileSegConnOrganizer_angle(segAngleMap, connectedSegs, elongConnMap);
	for (map<int, int>::iterator it = elongConnMap.begin(); it != elongConnMap.end(); ++it)
	{
		currProfiledTree.segs[it->first].to_be_deted = true;
		currProfiledTree.segs[it->second].to_be_deted = true;
		segUnit newSeg = this->segUnitConnect(currProfiledTree.segs[it->first], currProfiledTree.segs[it->second], tail_tail);
		newSeg.segID = currProfiledTree.segs.size() + 1;
		currProfiledTree.segs.insert(pair<int, segUnit>(newSeg.segID, newSeg));
		newSegs.insert(pair<int, segUnit>(newSeg.segID, newSeg));
	}
	segAngleMap.clear();
	elongConnMap.clear();
	cout << endl << endl;
	//__________________________________________________________________________________________________________________________

	return newSegs;
}

NeuronTree NeuronStructExplorer::segElongate(const profiledTree& inputProfiledTree)
{
	cout << inputProfiledTree.segHeadMap.size() << " " << inputProfiledTree.segTailMap.size() << endl;

	profiledTree outputProfiledTree = inputProfiledTree;
	QList<NeuronSWC> newNodeList = inputProfiledTree.tree.listNeuron;
	map<int, segUnit> allNewSegs;
	
	set<string> commonTileKeys;
	set<string> allTileKeys;
	for (map<string, vector<int>>::const_iterator headTileIt = inputProfiledTree.segHeadMap.begin(); headTileIt != inputProfiledTree.segHeadMap.end(); ++headTileIt)
		allTileKeys.insert(headTileIt->first);
	for (map<string, vector<int>>::const_iterator tailTileIt = inputProfiledTree.segTailMap.begin(); tailTileIt != inputProfiledTree.segTailMap.end(); ++tailTileIt)
		allTileKeys.insert(tailTileIt->first);
	//cout << commonTileKeys.size() << endl;
	
	vector<int> currTileHeadSegIDs;
	vector<int> currTileTailSegIDs;
	for (set<string>::iterator keyIt = allTileKeys.begin(); keyIt != allTileKeys.end(); ++keyIt)
	{
		cout << "TILE " << *keyIt << " =======" << endl;
		cout << " - Heads: ";
		if (inputProfiledTree.segHeadMap.find(*keyIt) != inputProfiledTree.segHeadMap.end())
		{
			for (vector<int>::const_iterator headIt = inputProfiledTree.segHeadMap.at(*keyIt).begin(); headIt != inputProfiledTree.segHeadMap.at(*keyIt).end(); ++headIt)
			{
				cout << *headIt << " ";
				currTileHeadSegIDs.push_back(*headIt);
			}
		}
		cout << endl;
		cout << " - Tails: ";
		if (inputProfiledTree.segTailMap.find(*keyIt) != inputProfiledTree.segTailMap.end())
		{
			for (vector<int>::const_iterator tailIt = inputProfiledTree.segTailMap.at(*keyIt).begin(); tailIt != inputProfiledTree.segTailMap.at(*keyIt).end(); ++tailIt)
			{
				cout << *tailIt << " ";
				currTileTailSegIDs.push_back(*tailIt);
			}
		}
		cout << endl;
		if (currTileHeadSegIDs.size() + currTileTailSegIDs.size() <= 1)
		{
			currTileHeadSegIDs.clear();
			currTileTailSegIDs.clear();
			cout << endl;
			continue;
		}

		map<int, segUnit> newSegs = this->segRegionConn_angle(currTileHeadSegIDs, currTileTailSegIDs, outputProfiledTree);
		for (map<int, segUnit>::iterator newSegIt = newSegs.begin(); newSegIt != newSegs.end(); ++newSegIt)
			allNewSegs.insert(pair<int, segUnit>(newSegIt->first, newSegIt->second));

		currTileHeadSegIDs.clear();
		currTileTailSegIDs.clear();
	}

	vector<size_t> nodeDeleteLocs;
	bool segDeleted = true;
	while (segDeleted)
	{
		for (map<int, segUnit>::iterator it = outputProfiledTree.segs.begin(); it != outputProfiledTree.segs.end(); ++it)
		{
			if (it->second.to_be_deted)
			{
				for (QList<NeuronSWC>::iterator nodeIt = it->second.nodes.begin(); nodeIt != it->second.nodes.end(); ++nodeIt)
					nodeDeleteLocs.push_back(outputProfiledTree.node2LocMap.at(nodeIt->n));

				outputProfiledTree.segs.erase(it);
				goto SEG_DELETED;
			}
		}
		segDeleted = false;

	SEG_DELETED:
		continue;
	}

	sort(nodeDeleteLocs.rbegin(), nodeDeleteLocs.rend());
	for (vector<size_t>::iterator it = nodeDeleteLocs.begin(); it != nodeDeleteLocs.end(); ++it) newNodeList.erase(newNodeList.begin() + ptrdiff_t(*it));

	outputProfiledTree.tree.listNeuron = newNodeList;
	for (map<int, segUnit>::iterator it = allNewSegs.begin(); it != allNewSegs.end(); ++it) outputProfiledTree.tree.listNeuron.append(it->second.nodes);

	return outputProfiledTree.tree;
}

vector<segUnit> NeuronStructExplorer::MSTtreeTrim(vector<segUnit>& inputSegUnits)
{
	vector<segUnit> outputSegUnits = inputSegUnits;

	for (vector<segUnit>::iterator segIt = outputSegUnits.begin(); segIt != outputSegUnits.end(); ++segIt)
	{
		if (segIt->nodes.size() <= 3) continue;

		NeuronStructUtil::node2loc_node2childLocMap(segIt->nodes, segIt->seg_nodeLocMap, segIt->seg_childLocMap);
		for (QList<NeuronSWC>::iterator nodeIt = segIt->nodes.begin(); nodeIt != segIt->nodes.end(); ++nodeIt)
		{
			if (segIt->seg_childLocMap[nodeIt->n].size() == 2)
			{
				if (nodeIt->parent == -1) continue;

				topoCharacter newTopoCenter(*nodeIt);
				newTopoCenter.topoCenterPa = segIt->nodes.at(segIt->seg_nodeLocMap[nodeIt->parent]);
				for (vector<size_t>::iterator immedChildLocIt = segIt->seg_childLocMap[nodeIt->n].begin(); immedChildLocIt != segIt->seg_childLocMap[nodeIt->n].end(); ++immedChildLocIt)
					newTopoCenter.topoCenterImmedChildren.push_back(segIt->nodes.at(*immedChildLocIt));

				if (segIt->seg_childLocMap[newTopoCenter.topoCenterImmedChildren.at(0).n].size() == 0 || segIt->seg_childLocMap[newTopoCenter.topoCenterImmedChildren.at(1).n].size() == 0)
					continue;
				
				double dot = (newTopoCenter.topoCenterImmedChildren.at(0).x - nodeIt->x) * (newTopoCenter.topoCenterImmedChildren.at(1).x - nodeIt->x) +
							 (newTopoCenter.topoCenterImmedChildren.at(0).y - nodeIt->y) * (newTopoCenter.topoCenterImmedChildren.at(1).y - nodeIt->y) +
							 (newTopoCenter.topoCenterImmedChildren.at(0).z - nodeIt->z) * (newTopoCenter.topoCenterImmedChildren.at(1).z - nodeIt->z);

				double sq1 = (newTopoCenter.topoCenterImmedChildren.at(0).x - nodeIt->x) * (newTopoCenter.topoCenterImmedChildren.at(0).x - nodeIt->x) +
							 (newTopoCenter.topoCenterImmedChildren.at(0).y - nodeIt->y) * (newTopoCenter.topoCenterImmedChildren.at(0).y - nodeIt->y) +
							 (newTopoCenter.topoCenterImmedChildren.at(0).z - nodeIt->z) * (newTopoCenter.topoCenterImmedChildren.at(0).z - nodeIt->z);

				double sq2 = (newTopoCenter.topoCenterImmedChildren.at(1).x - nodeIt->x) * (newTopoCenter.topoCenterImmedChildren.at(1).x - nodeIt->x) +
							 (newTopoCenter.topoCenterImmedChildren.at(1).y - nodeIt->y) * (newTopoCenter.topoCenterImmedChildren.at(1).y - nodeIt->y) +
							 (newTopoCenter.topoCenterImmedChildren.at(1).z - nodeIt->z) * (newTopoCenter.topoCenterImmedChildren.at(1).z - nodeIt->z);

				newTopoCenter.childAngle = acos(dot / sqrt(sq1 * sq2));
				if (isnan(acos(dot / sqrt(sq1 * sq2)))) newTopoCenter.childAngle = -1;

				newTopoCenter.immedChildrenLengths.insert(pair<int, double>(newTopoCenter.topoCenterImmedChildren.at(0).n, sqrt(sq1)));
				newTopoCenter.immedChildrenLengths.insert(pair<int, double>(newTopoCenter.topoCenterImmedChildren.at(1).n, sqrt(sq2)));

				segIt->topoCenters.push_back(newTopoCenter);
			}
		}
	}

	for (vector<segUnit>::iterator segIt = outputSegUnits.begin(); segIt != outputSegUnits.end(); ++segIt)
	{
		for (vector<topoCharacter>::iterator topoIt = segIt->topoCenters.begin(); topoIt != segIt->topoCenters.end(); ++topoIt)
		{
			if (topoIt->childAngle / PI <= 0.55 && topoIt->childAngle / PI >= 0.45) segIt->nodes[segIt->seg_nodeLocMap[topoIt->topoCenter.n]].type = 3;
		}
	}

	return outputSegUnits;

	/*if (inputSeg.size() <= 3) return;

#define PI 3.1415926
	//long segInfoNodeCount = 0;

	double dot, sq1, sq2, dist, turnCost, radAngle, turnCostMean = 0;
	double mainSqr = ((inputSeg.end() - 1)->x - inputSeg.begin()->x)*((inputSeg.end() - 1)->x - inputSeg.begin()->x) +
		((inputSeg.end() - 1)->y - inputSeg.begin()->y)*((inputSeg.end() - 1)->y - inputSeg.begin()->y) +
		((inputSeg.end() - 1)->z - inputSeg.begin()->z)*((inputSeg.end() - 1)->z - inputSeg.begin()->z);
	double segDist = sqrt(mainSqr);
	double nodeHeadDot, nodeHeadSqr, nodeHeadRadAngle, nodeToMainDist, nodeToMainDistMean = 0;
	//cout << "segment displacement: " << segDist << endl << endl;

	cout << "    -- nodes contained in the segment to be straightned: " << segInfoPtr->nodeCount << endl;
	for (vector<V_NeuronSWC_unit>::iterator check = inputSeg.begin(); check != inputSeg.end(); ++check) cout << "[" << check->x << " " << check->y << " " << check->z << "] ";
	cout << endl;
	vector<nodeInfo> pickedNode;
	for (vector<V_NeuronSWC_unit>::iterator it = inputSeg.begin() + 1; it != inputSeg.end() - 1; ++it)
	{
		dot = ((it - 1)->x - it->x)*((it + 1)->x - it->x) + ((it - 1)->y - it->y)*((it + 1)->y - it->y) + ((it - 1)->z - it->z)*((it + 1)->z - it->z);
		sq1 = ((it - 1)->x - it->x)*((it - 1)->x - it->x) + ((it - 1)->y - it->y)*((it - 1)->y - it->y) + ((it - 1)->z - it->z)*((it - 1)->z - it->z);
		sq2 = ((it + 1)->x - it->x)*((it + 1)->x - it->x) + ((it + 1)->y - it->y)*((it + 1)->y - it->y) + ((it + 1)->z - it->z)*((it + 1)->z - it->z);
		if (isnan(acos(dot / sqrt(sq1*sq2)))) return;
		radAngle = acos(dot / sqrt(sq1*sq2));
		nodeHeadDot = (it->x - inputSeg.begin()->x)*((inputSeg.end() - 1)->x - inputSeg.begin()->x) +
			(it->y - inputSeg.begin()->y)*((inputSeg.end() - 1)->y - inputSeg.begin()->y) +
			(it->z - inputSeg.begin()->z)*((inputSeg.end() - 1)->z - inputSeg.begin()->z);
		nodeHeadSqr = (it->x - inputSeg.begin()->x)*(it->x - inputSeg.begin()->x) + (it->y - inputSeg.begin()->y)*(it->y - inputSeg.begin()->y) + (it->z - inputSeg.begin()->z)*(it->z - inputSeg.begin()->z);
		nodeHeadRadAngle = PI - acos(nodeHeadDot / sqrt(mainSqr*nodeHeadSqr));
		nodeToMainDist = sqrt(nodeHeadSqr) * sin(nodeHeadRadAngle);
		nodeToMainDistMean = nodeToMainDistMean + nodeToMainDist;
		cout << "       d(node-main):" << nodeToMainDist << " radian/pi:" << (radAngle / PI) << " turning cost:" << (sqrt(sq1) + sqrt(sq2)) / (radAngle / PI) << " " << it->x << " " << it->y << " " << it->z << endl;

		if ((radAngle / PI) < 0.6) // Detecting sharp turns and distance outliers => a) obviously errorneous depth situation
		{
			nodeInfo sharp;
			sharp.x = it->x; sharp.y = it->y; sharp.z = it->z;
			cout << "this node is picked" << endl;

			sharp.segID = it->seg_id;
			sharp.distToMain = nodeToMainDist;
			sharp.sqr1 = sq1; sharp.sqr2 = sq2; sharp.dot = dot;
			sharp.radAngle = radAngle;
			sharp.nodeAddress = it;
			sharp.turnCost = (sqrt(sq1) + sqrt(sq2)) / (radAngle / PI);

			pickedNode.push_back(sharp);
			turnCostMean = turnCostMean + sharp.turnCost;
		}
	}
	if (pickedNode.empty()) return;

	nodeToMainDistMean = nodeToMainDistMean / (inputSeg.size() - 2);
	turnCostMean = turnCostMean / pickedNode.size();
	cout << endl << endl << "  ==== start deleting nodes... " << endl;

	int delete_count = 0;
	ptrdiff_t order = 0;
	vector<V_NeuronSWC> preservedSegs;
	for (vector<nodeInfo>::iterator it = pickedNode.begin(); it != pickedNode.end(); ++it)
	{
		cout << "  Avg(d(node_main)):" << nodeToMainDistMean << " d(node-main):" << it->distToMain << " Avg(turning cost):" << turnCostMean << " turning cost:" << it->turnCost;
		cout << " [" << it->x << " " << it->y << " " << it->z << "] " << endl;
		if (it->distToMain >= nodeToMainDistMean || it->turnCost >= turnCostMean || it->distToMain >= segDist)
		{
			++delete_count;
			if (connectEdit == segmentEdit) curImgPtr->tracedNeuron.seg[it->segID].to_be_deleted = true;
			--(segInfoPtr->nodeCount);

			V_NeuronSWC preservedNode;
			preservedNode.row.push_back(*(it->nodeAddress - order));
			preservedNode.row[0].data[6] = -1;
			preservedSegs.push_back(preservedNode);
			cout << "delete [" << (it->nodeAddress - order)->x << " " << (it->nodeAddress - order)->y << " " << (it->nodeAddress - order)->z << "] " << delete_count << endl;
			inputSeg.erase(it->nodeAddress - order);
			++order;
			cout << "----" << endl;
		}
	}

	cout << endl << "  ==== cheking angles... " << endl;

	int deleteCount2;
	do
	{
		deleteCount2 = 0;
		for (vector<V_NeuronSWC_unit>::iterator it = inputSeg.begin() + 1; it != inputSeg.end() - 1; ++it)
		{
			double dot2 = (it->x - (it - 1)->x)*((it + 1)->x - it->x) + (it->y - (it - 1)->y)*((it + 1)->y - it->y) + (it->z - (it - 1)->z)*((it + 1)->z - it->z);
			double sq1_2 = ((it - 1)->x - it->x)*((it - 1)->x - it->x) + ((it - 1)->y - it->y)*((it - 1)->y - it->y) + ((it - 1)->z - it->z)*((it - 1)->z - it->z);
			double sq2_2 = ((it + 1)->x - it->x)*((it + 1)->x - it->x) + ((it + 1)->y - it->y)*((it + 1)->y - it->y) + ((it + 1)->z - it->z)*((it + 1)->z - it->z);
			if (isnan(acos(dot2 / sqrt(sq1_2*sq2_2)))) break;
			double radAngle_2 = acos(dot2 / sqrt(sq1_2*sq2_2));
			cout << "2nd rad Angle:" << radAngle_2 << " [" << it->x << " " << it->y << " " << it->z << "]" << endl;

			if ((radAngle_2 / PI) * 180 > 75)
			{
				if (sqrt(sq1_2) > (1 / 10)*sqrt(sq2_2))
				{
					--(segInfoPtr->nodeCount);
					++deleteCount2;
					cout << "delete " << " [" << it->x << " " << it->y << " " << it->z << "] " << deleteCount2 << endl;
					if ((inputSeg.size() - deleteCount2) <= 2)
					{
						--deleteCount2;
						++(segInfoPtr->nodeCount);
						break;
					}
					if (connectEdit == segmentEdit) curImgPtr->tracedNeuron.seg[it->seg_id].to_be_deleted = true;

					V_NeuronSWC preservedNode;
					preservedNode.row.push_back(*it);
					preservedNode.row[0].data[6] = -1;
					preservedSegs.push_back(preservedNode);
					inputSeg.erase(it);
				}
			}
		}
		cout << "deleted nodes: " << deleteCount2 << "\n=================" << endl;
	} while (deleteCount2 > 0);

	size_t label = 1;
	cout << "number of nodes after straightening process: " << inputSeg.size() << " ( segID = " << segInfoPtr->segID << " )" << endl;
	//cout << "seg num: " << curImgPtr->tracedNeuron.seg.size() << endl;
	for (vector<V_NeuronSWC_unit>::iterator it = inputSeg.begin(); it != inputSeg.end(); ++it)
	{
		it->seg_id = segInfoPtr->segID;
		it->data[0] = label;
		it->data[6] = label + 1;
		++label;
		cout << "[" << it->seg_id << ": " << it->x << " " << it->y << " " << it->z << "] ";
	}
	cout << endl;
	(inputSeg.end() - 1)->data[6] = -1;

	if (connectEdit == segmentEdit)
	{
		V_NeuronSWC newSeg;
		newSeg.row = inputSeg;
		curImgPtr->tracedNeuron.seg[segInfoPtr->segID] = newSeg;

		size_t singleNodeCount = 1;
		for (vector<V_NeuronSWC>::iterator nodeIt = preservedSegs.begin(); nodeIt != preservedSegs.end(); ++nodeIt)
		{
			nodeIt->row[0].seg_id = curImgPtr->tracedNeuron.seg.size() + singleNodeCount;
			++singleNodeCount;
			curImgPtr->tracedNeuron.seg.push_back(*nodeIt);
			//cout << "seg num: " << curImgPtr->tracedNeuron.seg.size() << endl;
		}
	}

	return;*/
}

void NeuronStructExplorer::detectedPixelStackZProfile(NeuronTree* inputTreePtr, NeuronTree* outputTreePtr)
{
	// -- This method creates a z-frequency profile on the xy plane for the image stack.

	for (QList<NeuronSWC>::iterator it = inputTreePtr->listNeuron.begin(); it != inputTreePtr->listNeuron.end(); ++it)
	{
		int xCoord = int(it->x);
		int yCoord = int(it->y);
		int zCoord = int(it->z);
		float prob = it->radius;
		
		string xyKey = to_string(xCoord) + "," + to_string(yCoord);
		this->zProfileMap[xyKey][zCoord] = prob;
	}
}

void NeuronStructExplorer::falsePositiveList(NeuronTree* detectedTreePtr, NeuronTree* manualTreePtr, float distThreshold)
{
	// -- Determing false positives out of the detected structure by the threshold of specified distance to the manual structure.
	// -- if (the shortest distance of node i from detectedTree to manualTree > distThreshold) node i is deemed false positive.

	long int nodeCount = 1;
	for (QList<NeuronSWC>::iterator it = detectedTreePtr->listNeuron.begin(); it != detectedTreePtr->listNeuron.end(); ++it)
	{
		cout << "Scanning detected node " << nodeCount << ".. " << endl;
		++nodeCount;

		QList<NeuronSWC>::iterator checkIt = manualTreePtr->listNeuron.begin();
		while (checkIt != manualTreePtr->listNeuron.end())
		{
			float xSquare = (it->x - checkIt->x) * (it->x - checkIt->x);
			float ySquare = (it->y - checkIt->y) * (it->y - checkIt->y);
			float zSquare = (it->z - checkIt->z) * (it->z - checkIt->z);
			float thisNodeDist = sqrtf(xSquare + ySquare + zSquare);

			if (thisNodeDist < distThreshold)
			{
				this->processedTree.listNeuron.push_back(*it);
				break;
			}

			++checkIt;
			if (checkIt == manualTreePtr->listNeuron.end())
			{
				vector<float> FPcoords;
				FPcoords.push_back(it->x);
				FPcoords.push_back(it->y);
				FPcoords.push_back(it->z);
				this->FPsList.push_back(FPcoords);
				FPcoords.clear();
			}
		}
	}
}

void NeuronStructExplorer::falseNegativeList(NeuronTree* detectedTreePtr, NeuronTree* manualTreePtr, float distThreshold)
{
	// -- False negavies are selected out of the manual structure if none of detected nodes are close enough to them.
	// -- if (the shortest distance of node i from manualTree to detectedTree > distThreshold) node i is deemed false negative.

	long int nodeCount = 1;
	for (QList<NeuronSWC>::iterator it = manualTreePtr->listNeuron.begin(); it != manualTreePtr->listNeuron.end(); ++it)
	{
		cout << "Scanning detected node " << nodeCount << ".. " << endl;
		++nodeCount;

		QList<NeuronSWC>::iterator checkIt = detectedTreePtr->listNeuron.begin();
		while (checkIt != detectedTreePtr->listNeuron.end())
		{
			float xSquare = (it->x - checkIt->x) * (it->x - checkIt->x);
			float ySquare = (it->y - checkIt->y) * (it->y - checkIt->y);
			float zSquare = (it->z - checkIt->z) * (it->z - checkIt->z);
			float thisNodeDist = sqrtf(xSquare + ySquare + zSquare);

			if (thisNodeDist > distThreshold)
			{
				vector<float> FNcoords;
				FNcoords.push_back(it->x);
				FNcoords.push_back(it->y);
				FNcoords.push_back(it->z);
				this->FNsList.push_back(FNcoords);
				FNcoords.clear();

				break;
			}

			++checkIt;
		}
	}
}

void NeuronStructExplorer::detectedDist(NeuronTree* inputTreePtr1, NeuronTree* inputTreePtr2)
{
	// -- Compute the shortest distance of each node from input structure (inputTreePtr1) to refernce structure (inputTreePtr2) 
	// -- The order of inputTreePtr1 and inputTreePtr2 represent the distances from opposite comparing directions.

	vector<float> minDists;
	long int count = 1;
	for (QList<NeuronSWC>::iterator it1 = inputTreePtr1->listNeuron.begin(); it1 != inputTreePtr1->listNeuron.end(); ++it1)
	{
		++count;
		//cout << "node No. " << count << ".." << endl;
		float minDist = 10000;
		for (QList<NeuronSWC>::iterator it2 = inputTreePtr2->listNeuron.begin(); it2 != inputTreePtr2->listNeuron.end(); ++it2)
		{
			float xSquare = ((it2->x) - (it1->x)) * ((it2->x) - (it1->x));
			float ySquare = ((it2->y) - (it1->y)) * ((it2->y) - (it1->y));
			float zSquare = ((it2->z) - (it1->z)) * ((it2->z) - (it1->z));
			float currDist = sqrtf(xSquare + ySquare + zSquare);

			if (currDist <= minDist) minDist = currDist;
		}
		minDists.push_back(minDist);
	}

	float distSum = 0;
	for (vector<float>::iterator it = minDists.begin(); it != minDists.end(); ++it) distSum = distSum + *it;
	float distMean = distSum / minDists.size();
	float varSum = 0;
	for (int i = 0; i < minDists.size(); ++i) varSum = varSum + (minDists[i] - distMean) * (minDists[i] - distMean);
	float distVar = varSum / minDists.size();
	float distStd = sqrtf(distVar);
	
	cout << distMean << " " << distStd << endl;
}

void NeuronStructExplorer::shortestDistCDF(NeuronTree* inputTreePtr1, NeuronTree* inputTreePtr2, int upperBound, int binNum)
{
	// -- Compute the PDF and CDF of shortest distance of detected nodes to manual nodes.

	float interval = float(upperBound) / float(binNum);
	vector<float> shortestDists;
	for (QList<NeuronSWC>::iterator it1 = inputTreePtr1->listNeuron.begin(); it1 != inputTreePtr1->listNeuron.end(); ++it1)
	{
		float minDist = 10000;
		for (QList<NeuronSWC>::iterator it2 = inputTreePtr2->listNeuron.begin(); it2 != inputTreePtr2->listNeuron.end(); ++it2)
		{
			float xSquare = ((it2->x) - (it1->x)) * ((it2->x) - (it1->x));
			float ySquare = ((it2->y) - (it1->y)) * ((it2->y) - (it1->y));
			float zSquare = ((it2->z) - (it1->z)) * ((it2->z) - (it1->z));
			float currDist = sqrtf(xSquare + ySquare + zSquare);

			if (currDist <= minDist) minDist = currDist;
		}
		shortestDists.push_back(minDist);
	}

	for (int i = 0; i < binNum; ++i)
	{
		this->nodeDistPDF[i] = 0;
		this->nodeDistPDF[i] = 0;
	}
	for (vector<float>::iterator distIt = shortestDists.begin(); distIt != shortestDists.end(); ++distIt)
	{
		int binNo = (*distIt) / interval;
		++(this->nodeDistPDF[binNo]);
	}
	this->nodeDistCDF[0] = this->nodeDistPDF[0];
	for (int i = 1; i < binNum; ++i) this->nodeDistCDF[i] = this->nodeDistCDF[i - 1] + nodeDistPDF[i];
}

void NeuronStructExplorer::segmentDecompose(NeuronTree* inputTreePtr)
{
	// -- This function used get_link_map and decompose in v_neuronswc.cpp to get segment hierarchical information.

	this->segmentList.clear();
	this->segmentList = NeuronTree__2__V_NeuronSWC_list(*inputTreePtr);
}