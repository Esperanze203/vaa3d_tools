#include <iostream>
#include <fstream>
#include <sstream>
#include <ctime>
#include <iterator>
#include <map>
#include <string>

#include <boost\filesystem.hpp>
#include <boost/algorithm/string.hpp>

#include "SWCtester.h"
#include "ImgManager.h"
#include "ImgProcessor.h"

using namespace std;
using namespace boost;

int main(int argc, char* argv[])
{
	/********* specify function *********/
	const char* funcNameC = argv[1];
	//string funcName(funcNameC);
	string funcName = "interAccuracy";
	/************************************/

	if (!funcName.compare("2DblobMerge"))
	{
		//const char* inputSWCnameC = argv[2];
		//string inputSWCname(inputSWCnameC);
		string inputSWCname = "Z:\\IVSCC_mouse_inhibitory_442_swcLumps_2Dlabel\\478293723.swc";
		QString inputSWCnameQ = QString::fromStdString(inputSWCname);

		SWCtester mySWCtester;
		vector<connectedComponent> outputConnCompList = mySWCtester.connComponent2DmergeTest(inputSWCnameQ);
	}
	else if (!funcName.compare("swcID"))
	{
		string refSWCname = "H:\\IVSCC_mouse_inhibitory_442_swcROIcropped\\319215569.swc";
		string subjSWCname = "H:\\IVSCC_mouse_inhibitory_442_swcROIcropped_centroids3D\\319215569.swc";
		QString refSWCnameQ = QString::fromStdString(refSWCname);
		QString subjSWCnameQ = QString::fromStdString(subjSWCname);
		NeuronTree refTree = readSWC_file(refSWCnameQ);
		NeuronTree subjTree = readSWC_file(subjSWCnameQ);
		NeuronTree outputTree = NeuronStructUtil::swcIdentityCompare(subjTree, refTree, 50, 20);
		QString outputSWCname = "H:\\testOutput\\test.swc";
		writeSWC_file(outputSWCname, outputTree);
	}
	else if (!funcName.compare("cleanUpZ"))
	{
		string inputSWCname = "H:\\IVSCC_mouse_inhibitory\\442_swcROIcropped_centroids2D_diffTree\\319215569.swc";
		QString inputSWCnameQ = QString::fromStdString(inputSWCname);
		NeuronTree inputTree = readSWC_file(inputSWCnameQ);
		NeuronTree outputTree = NeuronStructUtil::swcZclenUP(inputTree);
		QString outputSWCname = "H:\\IVSCC_mouse_inhibitory\\testOutput\\test.swc";
		writeSWC_file(outputSWCname, outputTree);
	}
	else if (!funcName.compare("morphCheck"))
	{
		NeuronStructExplorer mySWCanalyzer;
		string inputSWCname = "Z:\\IVSCC_mouse_inhibitory\\442_swcROIcropped_centroids2D_zCleaned_MSTcut_zRatio\\319215569.swc";
		QString inputSWCnameQ = QString::fromStdString(inputSWCname);
		NeuronTree inputTree = readSWC_file(inputSWCnameQ);
		profiledTree currTree(inputTree);
		/*vector<segUnit> segs = mySWCanalyzer.MSTtreeTrim(currTree.segs);
		NeuronTree outputTree;
		for (vector<segUnit>::iterator it = segs.begin(); it != segs.end(); ++it)
		{
			for (QList<NeuronSWC>::iterator nodeIt = it->nodes.begin(); nodeIt != it->nodes.end(); ++nodeIt)
				outputTree.listNeuron.push_back(*nodeIt);
		}
		QString outputSWCname = "Z:\\IVSCC_mouse_inhibitory\\testOutput\\testMSTzCutMorph.swc";
		writeSWC_file(outputSWCname, outputTree);*/
	}
	else if (!funcName.compare("profiledTreeTest"))
	{
		string inputSWCname = "Z:\\IVSCC_mouse_inhibitory\\442_swcROIcropped_centroids2D_zCleaned_MSTcut\\319215569.swc";
		QString inputSWCnameQ = QString::fromStdString(inputSWCname);
		NeuronTree inputTree = readSWC_file(inputSWCnameQ);
		
		profiledTree testTree(inputTree);
		NeuronTree outputTree;
		/*for (vector<segUnit>::iterator it = testTree.segs.begin(); it != testTree.segs.end(); ++it)
		{
			for (QList<NeuronSWC>::iterator nodeIt = it->nodes.begin(); nodeIt != it->nodes.end(); ++nodeIt)
				outputTree.listNeuron.push_back(*nodeIt);
		}
		
		QString outputSWCname = "Z:\\IVSCC_mouse_inhibitory\\testOutput2\\test2.swc";
		writeSWC_file(outputSWCname, outputTree);*/
	}
	else if (!funcName.compare("MSTtest"))
	{
		string inputSWCname = "Z:\\IVSCC_mouse_inhibitory\\442_swcROIcropped_centroids2D_diffTree_zCleaned\\319215569.swc";
		QString inputSWCnameQ = QString::fromStdString(inputSWCname);
		NeuronTree inputTree = readSWC_file(inputSWCnameQ);

		NeuronStructExplorer mySWCanalyzer;
		NeuronTree outputTree = mySWCanalyzer.SWC2MSTtree(inputTree);
		QString outputSWCname = "Z:\\IVSCC_mouse_inhibitory\\testOutput2\\testMSTz.swc";
		writeSWC_file(outputSWCname, outputTree);
	}
	else if (!funcName.compare("MSTcut"))
	{
		string inputSWCname = "Z:\\IVSCC_mouse_inhibitory\\testOutput2\\testMSTz.swc";
		QString inputSWCnameQ = QString::fromStdString(inputSWCname);
		NeuronTree inputTree = readSWC_file(inputSWCnameQ);

		NeuronTree outputTree = NeuronStructExplorer::MSTtreeCut(inputTree, 20);
		QString outputSWCname = "Z:\\IVSCC_mouse_inhibitory\\testOutput2\\testMSTzCut.swc";
		writeSWC_file(outputSWCname, outputTree);
	}
	else if (!funcName.compare("branchBreak"))
	{
		string inputSWCname = "Z:\\IVSCC_mouse_inhibitory\\442_swcROIcropped_centroids2D\\MST2nd\\319215569.swc";
		QString inputSWCnameQ = QString::fromStdString(inputSWCname);
		NeuronTree inputTree = readSWC_file(inputSWCnameQ);
		profiledTree inputProfiledTree(inputTree);
		NeuronTree outputTree = NeuronStructExplorer::MSTbranchBreak(inputProfiledTree, true);
		QString outputSWCname = "Z:\\IVSCC_mouse_inhibitory\\testOutput\\test.swc";
		writeSWC_file(outputSWCname, outputTree);
	}
	else if (!funcName.compare("segElongate"))
	{
		string inputSWCname = "H:\\IVSCC_mouse_inhibitory\\testInput\\319215569.swc";
		QString inputSWCnameQ = QString::fromStdString(inputSWCname);
		NeuronTree inputTree = readSWC_file(inputSWCnameQ);
		profiledTree inputProfiledTree(inputTree);

		NeuronStructExplorer mySWCExplorer;
		profiledTree elongatedTree = mySWCExplorer.itered_segElongate(inputProfiledTree);

		QString outputSWCname = "H:\\IVSCC_mouse_inhibitory\\testOutput\\test.swc";
		writeSWC_file(outputSWCname, elongatedTree.tree);
	}
	else if (!funcName.compare("dotRemove"))
	{
		string inputSWCname = "Z:\\IVSCC_mouse_inhibitory\\442_swcROIcropped_centroids2D\\MST2nd_branchBreak_noSpike_elong\\319215569.swc";
		QString inputSWCnameQ = QString::fromStdString(inputSWCname);
		NeuronTree inputTree = readSWC_file(inputSWCnameQ);
		profiledTree inputProfiledTree(inputTree);

		NeuronStructExplorer mySWCExplorer;
		NeuronTree outputTree = mySWCExplorer.singleDotRemove(inputProfiledTree);

		QString outputSWCname = "Z:\\IVSCC_mouse_inhibitory\\testOutput\\test.swc";
		writeSWC_file(outputSWCname, outputTree);
	}
	else if (!funcName.compare("treeUnion"))
	{
		string inputSWCname1 = "H:\\IVSCC_mouse_inhibitory\\442_swcROIcropped_centroids2D\\MST2nd_tiled30_branchBreak_noSpike_elong_noDots\\319215569.swc";
		string inputSWCname2 = "H:\\IVSCC_mouse_inhibitory\\442_swcROIcropped_centroids2D\\diffTree_zCleaned_MST_zRatio_branchBreak_noSpike_elong\\319215569.swc";
		QString inputSWCname1Q = QString::fromStdString(inputSWCname1);
		QString inputSWCname2Q = QString::fromStdString(inputSWCname2);
		NeuronTree inputTree1 = readSWC_file(inputSWCname1Q);
		NeuronTree inputTree2 = readSWC_file(inputSWCname2Q);
		profiledTree inputProfiledTree1(inputTree1);
		profiledTree inputProfiledTree2(inputTree2);

		NeuronStructExplorer mySWCExplorer;
		profiledTree outputProfiledTree = mySWCExplorer.treeUnion_MSTbased(inputProfiledTree1, inputProfiledTree2);

		QString outputSWCname = "H:\\IVSCC_mouse_inhibitory\\testOutput\\test.swc";
		writeSWC_file(outputSWCname, outputProfiledTree.tree);
	}
	else if (!funcName.compare("selfDist"))
	{
		ofstream outputTest("Z:\\IVSCC_mouse_inhibitory\\442_swcROIcropped_centroids2D\\test.txt");

		string inputSWCname1 = "Z:\\IVSCC_mouse_inhibitory\\442_swcROIcropped_centroids2D\\diffTree_zCleaned_MST_zRatio_branchBreak_noSpike_elong_noDotSeg2_tile30elong30_tile60elong30_RESULT\\319215569.swc";
		string inputSWCname2 = "Z:\\IVSCC_mouse_inhibitory\\442_swcROIcropped_centroids2D\\442_swcROIcropped\\319215569.swc";
		QString inputSWCnameQ1 = QString::fromStdString(inputSWCname1);
		QString inputSWCnameQ2 = QString::fromStdString(inputSWCname2);
		NeuronTree inputTree1 = readSWC_file(inputSWCnameQ1);
		NeuronTree inputTree2 = readSWC_file(inputSWCnameQ2);
		map<string, float> swcStats = NeuronStructUtil::selfNodeDist(inputTree1.listNeuron);
		
		outputTest << swcStats["mean"] << " " << swcStats["std"] << endl;
	}
	else if (!funcName.compare("accuracy"))
	{
		const char* inputPathNameC = argv[1];
		string inputPathName(inputPathNameC);
		ofstream outputFile("Z:\\IVSCC_mouse_inhibitory\\442_swcROIcropped_centroids2D\\accuracy.txt");
		outputFile << "case num\t" << "rate\t" << "node num" << endl;

		for (filesystem::directory_iterator swcIt(inputPathName); swcIt != filesystem::directory_iterator(); ++swcIt)
		{
			string swcName = swcIt->path().filename().string();
			string swcFullName = inputPathName + "\\" + swcName;
			QString swcFullNameQ = QString::fromStdString(swcFullName);
			NeuronTree nt = readSWC_file(swcFullNameQ);

			int TP = 0;
			int FP = 0;
			for (QList<NeuronSWC>::iterator it = nt.listNeuron.begin(); it != nt.listNeuron.end(); ++it)
			{
				if (it->type == 2) ++TP;
				else if (it->type == 3) ++FP;
			}
			double rate = double(TP) / double(TP + FP);

			string caseNum = swcName.substr(0, 9);
			outputFile << caseNum << "\t" << rate << "\t" << nt.listNeuron.size() << endl;
		}
	}
	else if (!funcName.compare("interAccuracy"))
	{
		const char* inputPathNameC = argv[1];
		string inputPathName(inputPathNameC);
		ofstream outputFile("Z:\\IVSCC_mouse_inhibitory\\442_swcROIcropped_centroids2D\\interAccuracy.txt");
		outputFile << "case num\t" << "avgDist 1-2\t" << "structure diff 1-2\t" << "avgDist 2-1\t" << "structure diff 2-1\t" << "avfDist all\t" << "structure diff all" << endl;

		for (filesystem::directory_iterator swcIt(inputPathName); swcIt != filesystem::directory_iterator(); ++swcIt)
		{
			string csvName = swcIt->path().filename().string();
			string csvFullName = inputPathName + "\\" + csvName;
			string caseName = csvName.substr(0, 9);
			//cout << csvFullName << endl;
			ifstream inputCSV(csvFullName);
			string inputLine;
			int count1_2 = 0;
			int count2_1 = 0;
			vector<string> values(7);
			values[0] = caseName;
			while (getline(inputCSV, inputLine))
			{
				if (inputLine.find("from neuron 1 to 2") != string::npos)
				{
					++count1_2;
					vector<string> inputs;
					boost::split(inputs, inputLine, boost::is_any_of(" "));
					//for (vector<string>::iterator checkit = inputs.begin(); checkit != inputs.end(); ++checkit) cout << *checkit << " ";
					//cout << inputs.size() << " ";
					string measure = inputs.back();			
					
					if (count1_2 == 1) values[1] = measure;
					else if (count1_2 == 2) values[2] = measure;
				}
				else if (inputLine.find("from neuron 2 to 1") != string::npos)
				{
					++count2_1;
					vector<string> inputs;
					boost::split(inputs, inputLine, boost::is_any_of(" "));
					//for (vector<string>::iterator checkit = inputs.begin(); checkit != inputs.end(); ++checkit) cout << *checkit << " ";
					//cout << inputs.size() << " ";
					string measure = inputs.back();

					if (count2_1 == 1) values[3] = measure;
					else if (count2_1 == 2) values[4] = measure;
				}
				else if (inputLine.find("structure-averages") != string::npos)
				{
					vector<string> inputs;
					boost::split(inputs, inputLine, boost::is_any_of(" "));
					//for (vector<string>::iterator checkit = inputs.begin(); checkit != inputs.end(); ++checkit) cout << *checkit << " ";
					//cout << inputs.size() << " ";
					string measure = inputs.back();
					values[5] = measure;
				}
				else if (inputLine.find("(average)") != string::npos)
				{
					vector<string> inputs;
					boost::split(inputs, inputLine, boost::is_any_of(" "));
					//for (vector<string>::iterator checkit = inputs.begin(); checkit != inputs.end(); ++checkit) cout << *checkit << " ";
					//cout << inputs.size() << " ";
					string measure = inputs.back();
					values[6] = measure;
				}
			}
			for (vector<string>::iterator it = values.begin(); it != values.end(); ++it) outputFile << *it << "\t";
			outputFile << endl;
			values.clear();
			inputLine.clear();
		}
	}
	else if (!funcName.compare("makeLinkerFile"))
	{
		const char* inputPathNameC = argv[1];
		string inputPathName(inputPathNameC);
		//ofstream outputFile("Z:\\IVSCC_mouse_inhibitory\\442_swcROIcropped_centroids2D\\interAccuracy.txt");
		//outputFile << "case num\t" << "avgDist\t" << "structure diff" << endl;

		for (filesystem::directory_iterator swcIt(inputPathName); swcIt != filesystem::directory_iterator(); ++swcIt)
		{
			string swcName = swcIt->path().filename().string();
			string swcFullName = inputPathName + "\\" + swcName;
			NeuronStructUtil::linkerFileGen_forSWC(swcFullName);
		}
	}
	else if (!funcName.compare("somaPointsCluster"))
	{
		const char* inputFileNameC = argv[1];
		string inputFileName = "Z:\\IVSCC_mouse_inhibitory\\442_swcROIcropped_somaCentroid3D\\472599722.swc";
		//string inputFileName(inputFileNameC);
		QString inputFileNameQ = QString::fromStdString(inputFileName);
		NeuronTree nt = readSWC_file(inputFileNameQ);

		vector<connectedComponent> compList = NeuronStructUtil::swc2clusters_distance(nt, 20);
		for (vector<connectedComponent>::iterator compIt = compList.begin(); compIt != compList.end(); ++compIt)
		{
			cout << "[";
			ImgAnalyzer::ChebyshevCenter_connComp(*compIt);
			for (map<int, set<vector<int>>>::iterator it = compIt->coordSets.begin(); it != compIt->coordSets.end(); ++it)
			{
				for (set<vector<int>>::iterator it2 = it->second.begin(); it2 != it->second.end(); ++it2)
				{
					cout << it2->at(0) << "_" << it2->at(1) << "_" << it2->at(2) << ", ";
				}
			}
			cout << "] ";
			cout << compIt->ChebyshevCenter[0] << " " << compIt->ChebyshevCenter[1] << " " << compIt->ChebyshevCenter[2] << endl;
		}
	}
	else if (!funcName.compare("multipleSomaCandidates"))
	{
		string inputFileName = "Z:\\IVSCC_mouse_inhibitory\\442_swcROIcropped_somaCandidates\\";
		vector<string> fileList;
		for (filesystem::directory_iterator fileIt(inputFileName); fileIt != filesystem::directory_iterator(); ++fileIt)
		{
			string fileName = fileIt->path().string();
			QString fileNameQ = QString::fromStdString(fileName);
			NeuronTree nt = readSWC_file(fileNameQ);

			if (nt.listNeuron.size() == 2) fileList.push_back(fileName);
		}

		for (vector<string>::iterator it = fileList.begin(); it != fileList.end(); ++it) cout << *it << endl;
	}
	else if (!funcName.compare("skeleton"))
	{
		string fileFullName = "Z:\\IVSCC_mouse_inhibitory\\442_max_thr_999_ROIcropped_MIP\\319215569.tif";
		string fileName = "319215569.tif";
		ImgManager myManager;
		myManager.inputSingleCaseSingleSliceFullPath = fileFullName;
		myManager.imgEntry(fileName, ImgManager::singleCase_singleSlice);
		int imgDims[3];
		imgDims[0] = myManager.imgDatabase.begin()->second.dims[0];
		imgDims[1] = myManager.imgDatabase.begin()->second.dims[1];
		imgDims[2] = 1;
		unsigned char* outputImgPtr = new unsigned char[imgDims[0] * imgDims[1]];
		ImgProcessor::skeleton2D(myManager.imgDatabase.begin()->second.slicePtrs.begin()->second.get(), outputImgPtr, imgDims);
		
		V3DLONG Dims[4];
		Dims[0] = imgDims[0];
		Dims[1] = imgDims[1];
		Dims[2] = 1;
		Dims[3] = 1;
		string sliceSaveFullName = "Z:\\IVSCC_mouse_inhibitory\\testOutput\\test.tif";
		const char* sliceSaveFullNameC = sliceSaveFullName.c_str();
		ImgManager::saveimage_wrapper(sliceSaveFullNameC, outputImgPtr, Dims, 1);

		delete[] outputImgPtr;
	}
	else if (!funcName.compare("skeletonSlices"))
	{
		const char* inputPathNameC = argv[1];
		string inputPathName(inputPathNameC);
		const char* outputPathNameC = argv[2];
		string outputPathName(outputPathNameC);
		//cout << outputPathName << endl;
		vector<string> inputFullPathParse;
		boost::split(inputFullPathParse, inputPathName, boost::is_any_of("/"));
		string caseNum = *(inputFullPathParse.end() - 1);
		QString caseNumQ = QString::fromStdString(caseNum);
		
		ImgManager myImgManager;
		for (filesystem::directory_iterator fileIt(inputPathName); fileIt != filesystem::directory_iterator(); ++fileIt)
		{
			string fileFullName = fileIt->path().string();
			myImgManager.inputMultiCasesSliceFullPaths.insert(pair<string, string>(caseNum, fileFullName));
		}
		
		myImgManager.imgEntry(caseNum, ImgManager::slices);
		int imgDims[3];
		imgDims[0] = myImgManager.imgDatabase.begin()->second.dims[0];
		imgDims[1] = myImgManager.imgDatabase.begin()->second.dims[1];
		imgDims[2] = 1;
		for (map<string, myImg1DPtr>::iterator it = myImgManager.imgDatabase.begin()->second.slicePtrs.begin(); it != myImgManager.imgDatabase.begin()->second.slicePtrs.end(); ++it)
		{
			unsigned char* outputImgPtr = new unsigned char[imgDims[0] * imgDims[1]];
			ImgProcessor::skeleton2D(it->second.get(), outputImgPtr, imgDims);

			V3DLONG Dims[4];
			Dims[0] = imgDims[0];
			Dims[1] = imgDims[1];
			Dims[2] = 1;
			Dims[3] = 1;
			string sliceSaveFullName = outputPathName + "\\" + it->first;
			cout << sliceSaveFullName << endl;
			const char* sliceSaveFullNameC = sliceSaveFullName.c_str();
			ImgManager::saveimage_wrapper(sliceSaveFullNameC, outputImgPtr, Dims, 1);

			delete[] outputImgPtr;
		}
	}
	else if (!funcName.compare("simpleBlend"))
	{
		const char* folder1C = argv[2];
		string folder1(folder1C);
		const char* folder2C = argv[3];
		string folder2(folder2C);

		for (filesystem::directory_iterator sliceIt(folder1); sliceIt != filesystem::directory_iterator(); ++sliceIt)
		{
			string caseName = sliceIt->path().filename().string();
			string caseNameFullPath = folder1 + "\\" + caseName;
			caseName = caseName.substr(0, 9);
			for (filesystem::directory_iterator sliceIt2(folder2); sliceIt2 != filesystem::directory_iterator(); ++sliceIt2)
			{
				string caseName2 = sliceIt2->path().filename().string();
				string caseName2FullPath = folder2 + "\\" + caseName2;
				caseName2 = caseName2.substr(0, 9);
				if (!caseName2.compare(caseName))
				{
					//cout << caseNameFullPath << endl << caseName2FullPath << endl;
					ImgManager myImgManager;
					myImgManager.inputSingleCaseSingleSliceFullPath = caseNameFullPath;
					myImgManager.imgEntry("img1", ImgManager::singleCase_singleSlice);
					myImgManager.inputSingleCaseSingleSliceFullPath = caseName2FullPath;
					myImgManager.imgEntry("img2", ImgManager::singleCase_singleSlice);

					vector<unsigned char*> blendingPtrs;
					for (map<string, registeredImg>::iterator it = myImgManager.imgDatabase.begin(); it != myImgManager.imgDatabase.end(); ++it)
						blendingPtrs.push_back(it->second.slicePtrs.begin()->second.get());

					unsigned char* imgArray1D = new unsigned char[myImgManager.imgDatabase.begin()->second.dims[0] * myImgManager.imgDatabase.begin()->second.dims[1] * 2];
					ImgManager::imgsBlend(blendingPtrs, imgArray1D, myImgManager.imgDatabase.begin()->second.dims);

					V3DLONG Dims[4];
					Dims[0] = myImgManager.imgDatabase.begin()->second.dims[0];
					Dims[1] = myImgManager.imgDatabase.begin()->second.dims[1];
					Dims[2] = 1;
					Dims[3] = 2;

					const char* saveNameC = argv[4];
					string saveName(saveNameC);
					saveName = saveName + "\\" + caseName2 + ".tif";
					const char* saveFullNameC = saveName.c_str();
					ImgManager::saveimage_wrapper(saveFullNameC, imgArray1D, Dims, 1);

					myImgManager.imgDatabase.clear();
				}
			}
		}
	}
	else if (!funcName.compare("getImgStats"))
	{
		const char* folderNameC = argv[1];
		string folderName(folderNameC);
		const char* outputFileNameC = argv[2];
		string outputFileName(outputFileNameC);

		ofstream outputFile(outputFileName);
		outputFile << "case\t";
		for (int i = 255; i >= 100; --i) outputFile << i << "\t";
		outputFile << endl;
		//outputFile << "case\tmean\tstd\tmedian" << endl;
		ImgManager myImgManager;
		/*for (filesystem::directory_iterator imgIt(folderName); imgIt != filesystem::directory_iterator(); ++imgIt)
		{
			string sliceFullName = imgIt->path().string();
			myImgManager.inputSingleCaseSingleSliceFullPath = sliceFullName;
			myImgManager.imgEntry(imgIt->path().filename().string(), ImgManager::singleCase_singleSlice);
			map<string, float> imgStats = ImgProcessor::getBasicStats_no0(myImgManager.imgDatabase.begin()->second.slicePtrs.begin()->second.get(), myImgManager.imgDatabase.begin()->second.dims);
			outputFile << imgIt->path().filename().string() << "\t" << imgStats.at("mean") << "\t" << imgStats.at("std") << "\t" << imgStats.at("median") << endl;

			myImgManager.imgDatabase.clear();
		}*/

		for (filesystem::directory_iterator imgIt(folderName); imgIt != filesystem::directory_iterator(); ++imgIt)
		{
			string sliceFullName = imgIt->path().string();
			myImgManager.inputSingleCaseSingleSliceFullPath = sliceFullName;
			myImgManager.imgEntry(imgIt->path().filename().string(), ImgManager::singleCase_singleSlice);
			map<int, size_t> histMap = ImgProcessor::histQuickList(myImgManager.imgDatabase.begin()->second.slicePtrs.begin()->second.get(), myImgManager.imgDatabase.begin()->second.dims);
			outputFile << imgIt->path().filename().string() << "\t";
			for (int j = 255; j >= 100; --j)
			{
				if (histMap.find(j) == histMap.end())
				{	
					if (j < 200) break;
					else continue;
				}
				else outputFile << histMap.at(j) << "\t";
			}
			outputFile << endl;

			myImgManager.imgDatabase.clear();
		}
	}
	else if (!funcName.compare("swcSubtract"))
	{
		//const char* targetSWCNameC = argv[1];
		//string targetSWCName(targetSWCNameC);
		QString targetSWCNameQ = "Z:\\IVSCC_mouse_inhibitory\\442_swcROIcropped_centroids2D\\319215569_RESULT.swc";
		//const char* refSWCNameC = argv[2];
		//string refSWCName(refSWCNameC);
		QString refSWCNameQ = "Z:\\IVSCC_mouse_inhibitory\\442_swcROIcropped_centroids2D\\319215569_denDiff.swc";
		//NeuronTree targetTree = readSWC_file(QString::fromStdString(targetSWCName));
		//NeuronTree refTree = readSWC_file(QString::fromStdString(refSWCName));
		NeuronTree targetTree = readSWC_file(targetSWCNameQ);
		NeuronTree refTree = readSWC_file(refSWCNameQ);

		NeuronTree subTree = NeuronStructUtil::swcSubtraction(targetTree, refTree, 2);
		QString saveName = "Z:\\IVSCC_mouse_inhibitory\\442_swcROIcropped_centroids2D\\test.swc";
		writeSWC_file(saveName, subTree);
	}
	else if (!funcName.compare("swcUpSample"))
	{
		//const char* targetSWCNameC = argv[2];
		//string targetSWCName(targetSWCNameC);
		//QString targetSWCNameQ = QString::fromStdString(targetSWCName);
		QString targetSWCNameQ = "H:\\IVSCC_mouse_inhibitory\\442_swcROIcropped_centroids2D\\319215569.swc";
		NeuronTree inputTree = readSWC_file(targetSWCNameQ);

		profiledTree inputProfiledTree(inputTree);
		profiledTree outputProfiledTree;
		outputProfiledTree.tree.listNeuron.clear();
		NeuronStructExplorer::treeUpSample(inputProfiledTree, outputProfiledTree);

		//const char* saveSWCNameC = argv[3];
		//string saveSWCName(saveSWCNameC);
		//QString saveSWCNameQ = QString::fromStdString(saveSWCName);
		QString saveSWCNameQ = "H:\\IVSCC_mouse_inhibitory\\442_swcROIcropped_centroids2D\\test_upsampled.swc";
		writeSWC_file(saveSWCNameQ, outputProfiledTree.tree);
	}

	return 0;
}


