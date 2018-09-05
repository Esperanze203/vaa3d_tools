#include "readmain.h"
#include <v3d_interface.h>
//#include "v3d_message.h"
#include "openSWCDialog.h"
#include <vector>
#include <iostream>
using namespace std;


bool export_list2file(QList<NeuronSWC> & newone, QString fileSaveName, QString fileOpenName)
{
    QFile file(fileSaveName);
    if (!file.open(QIODevice::WriteOnly|QIODevice::Text))
        return false;
    QTextStream myfile(&file);
    myfile<<"# generated by Vaa3D Plugin sort_neuron_swc"<<endl;
    myfile<<"# source file(s): "<<fileOpenName<<endl;
    myfile<<"# id,type,x,y,z,r,pid"<<endl;
    for (V3DLONG i=0;i<newone.size();i++)
        myfile << newone.at(i).n <<" " << newone.at(i).type << " "<< newone.at(i).x <<" "<<newone.at(i).y << " "<< newone.at(i).z << " "<< newone.at(i).r << " " <<newone.at(i).pn << "\n";

    file.close();
    cout<<"swc file "<<fileSaveName.toStdString()<<" has been generated, size: "<<newone.size()<<endl;
    return true;
}

void readtree(V3DPluginCallback2 &callback, QWidget *parent)
{
    OpenSWCDialog * openDlg = new OpenSWCDialog(0, &callback);
    if (openDlg->exec())
    {

    NeuronTree nt = openDlg->nt;
    QList<NeuronSWC> neuron = nt.listNeuron;
    qDebug()<<neuron.size();

    QList<NeuronSWC> newone;
    NeuronSWC S;
    S.n =neuron.at(0).n;
    S.pn = neuron.at(0).pn;
    S.x = neuron.at(0).x;
    S.y = neuron.at(0).y;
    S.z = neuron.at(0).z;
    S.r = neuron.at(0).r;
    S.type = neuron.at(0).type;
    newone.append(S);
     qDebug()<<newone.size();
    cout<<"===============================welcome==============================="<<endl;
    int i=1;
    while(neuron.at(i).pn>0)
        {
            NeuronSWC S;
            S.n =neuron.at(i).n;
            S.pn = neuron.at(i).pn;
            S.x = neuron.at(i).x;
            S.y = neuron.at(i).y;
            S.z = neuron.at(i).z;
            S.r = neuron.at(i).r;
            S.type = neuron.at(i).type;
            newone.append(S);
            i=i+1;
        }
    qDebug()<<newone.size();
    QString fileOpenName = openDlg->file_name;
    QString fileDefaultName = fileOpenName+QString("_singletree.swc");
        //write new SWC to file
    QString fileSaveName = QFileDialog::getSaveFileName(0, QObject::tr("Save File"),
            fileDefaultName,
            QObject::tr("Supported file (*.swc)"
                    ";;Neuron structure	(*.swc)"
                    ));
    if (fileSaveName.isEmpty()) return;
    if (!export_list2file(newone,fileSaveName,fileOpenName))
    {
        v3d_msg("fail to write the output swc file.");
        return;
    }
    }
 }
