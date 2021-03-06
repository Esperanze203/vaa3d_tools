#include <vector>
#include "branch_detect.h"

//#include "../../../../vaa3d_tools/released_plugins/v3d_plugins/neurontracing_vn2/app2/my_surf_objs.h"

#define dist(a,b) sqrt(((a).x-(b).x)*((a).x-(b).x)+((a).y-(b).y)*((a).y-(b).y)+((a).z-(b).z)*((a).z-(b).z))
#define PI 3.14159265359
#define MIN_DIST 10
#define VOID 1000000000
#ifndef SWC_RADIUS
#define SWC_RADIUS 5
#endif
#ifndef RX
#define RX 0.2
#endif
#ifndef RY
#define RY 0.2
#endif
#ifndef RZ
#define RZ 1
#endif



double marker_dist(MyMarker a, MyMarker b, bool scale)
{
    if(scale){
        a.x = a.x*RX;
        a.y = a.y*RY;
        a.z = a.z*RZ;
        b.x = b.x*RX;
        b.y = b.y*RY;
        b.z = b.z*RZ;
    }
    return sqrt((a.x - b.x)*(a.x - b.x) + (a.y - b.y)*(a.y - b.y) + (a.z - b.z)*(a.z - b.z));
}
using namespace std;


void get_branches(const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback)
{
    vector<char*> infiles, inparas, outfiles;
    if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
    if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
    if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

    QString image_file=infiles.at(1);
//    QString swc_file = infiles.at(1);
    QString output_dir=outfiles.at(0);

    QString swc_file = infiles.at(0);

    QStringList list=swc_file.split("/");
    QString flag=list.last(); QStringList list1=flag.split(".");// you don't need to add 1 to find the string you want in input_dir
    QString flag1=list1.first();
//    QString output_fake = output_dir+"/"+flag1+"_fake.apo";
    QString output_branch = output_dir+"/"+flag1+".apo";

    printf("welcome to use get_branch\n");
    NeuronTree nt = readSWC_file(swc_file);
//    if(!output_dir.endsWith("/")){
//        output_dir = output_dir+"/";
//    }
    //QString cell_name = swc_file.right(swc_file.size()-swc_file.lastIndexOf("/")-1);
    //cell_name = cell_name.left(cell_name.indexOf("."));
    //QString output_file=output_dir;

    //cout<<"image loading"<<endl;
    // Find branch points
    V3DLONG *in_zz;
    //cout<<"image file name is "<<image_file.toStdString()<<endl;
    if(!callback.getDimTeraFly(image_file.toStdString(), in_zz))
    {
//        v3d_msg("Cannot load terafly images.",0);
        cout<<"can not load terafly images"<<endl;
    }
    cout<<"check1"<<endl;
    XYZ block_size=XYZ(100,100,20);
    block zcenter_block; // This is a block centered at (0,0,0)
    zcenter_block.small = 0-block_size/2;
    zcenter_block.large = block_size/2;

    QList<int> branch_list;
    QList<int> plist;
    QList<int> alln;
    QList<NeuronSWC> apo_list;
    QList<NeuronSWC> apo_angle;
    QList<NeuronSWC> apo_dis;
    QList<NeuronSWC> apo_branch;
    QList<int> parent;
    int N=nt.listNeuron.size();
    map<int, int> t;
    QList<int> branch;
    map<int,int > child1;
    map<int,int > child2;
    int root_id;

    for(int i=0; i<N; i++){
        //qDebug() << nt.listNeuron.at(i).n << nt.listNeuron.at(i).pn;
        if(nt.listNeuron.at(i).pn==-1){
            root_id=i;
        }
        plist.append(nt.listNeuron.at(i).pn);
        alln.append(nt.listNeuron.at(i).n);
        t.insert(pair<int,int>(plist.at(i),0));
    }

    for(int i=0; i<N; i++){
        t.at(plist.at(i))=t.at(plist.at(i))+1;
        if((plist.count(plist.at(i))>1)&(t.at(plist.at(i)) == 1))
        {
            //cout<<"pruning"<<endl;

//            int p_index;
//            p_index = plist.indexOf(i);
//            if(nt.listNeuron.at(p_index).pn!=-1){
//            branch.push_back(p_index);
            if(nt.listNeuron.at(i).pn!=-1){
            branch.push_back(nt.listNeuron.at(i).pn);
            //parent.push_back(p_index);
            child1.insert(pair<int,int>(plist.at(i),alln.at(i)));
            }
         }
        else if(t.at(plist.at(i)) ==2){
            child2.insert(pair<int,int>(plist.at(i),alln.at(i)));
        }
        }

//    for(map<int,int>::iterator it=child1.begin();it!=child1.end();it++){
//        cout<<it->first<<":"<<it->second<<endl;}
//    for(map<int,int>::iterator it=child2.begin();it!=child2.end();it++){
//        cout<<it->first<<":"<<it->second<<endl;}

        //cout<<"size="<<branch.size()<<endl;
    double local_angle =0;
    int count=0;

    for(int i=0; i<branch.size(); i++){
            // p_index: line where branch point is in
            int p_index;
            p_index = alln.indexOf(branch[i]);
//            if(nt.listNeuron.at(p_index).pn!=-1){
//            cout<<"index"<<p_index<<endl;
            //double d = dist(nt.listNeuron.at(alln.at(i)),nt.listNeuron.at(p_index));
            //distance & intensity
        //int bid = alln.indexOf(plist.at(branch[i]));
        //cout<<plist.at(branch[i])<<" "<<branch[i]<<" "<<bid<<endl;
//            V3DLONG nodex = nt.listNeuron.at(branch[i]).x;
//            V3DLONG nodey = nt.listNeuron.at(branch[i]).y;
//            V3DLONG nodez = nt.listNeuron.at(branch[i]).z;
        float nodex = nt.listNeuron.at(p_index).x;
        float nodey = nt.listNeuron.at(p_index).y;
        float nodez = nt.listNeuron.at(p_index).z;

            cout<<"check2"<<endl;

            //cout<<"cordinates:"<<nodex<<" ,"<<nodey<<" ,"<<nodez<<endl;
            //block
            if(p_index != 0){
            block crop_block = offset_block(zcenter_block, XYZ(nodex, nodey, nodez));

            XYZ small=XYZ(crop_block.small);
            XYZ large=XYZ(crop_block.large);
            small.x = floor(small.x);
            small.y = floor(small.y);
            small.z = floor(small.z);
            large.x = ceil(large.x)+1;
            large.y = ceil(large.y)+1;
            large.z = ceil(large.z)+1;
            unsigned char * cropped_image = 0;
            //cout<<"dim"<<small.x<<small.y<<small.z<<large.x<<large.y<<large.z<<endl;
            cropped_image = callback.getSubVolumeTeraFly(image_file.toStdString(),
                                                         small.x, large.x,
                                                         small.y, large.y,
                                                         small.z, large.z);
//            cropped_image = callback.getSubVolumeTeraFly(image_file.toStdString(),
//                                                         10, 20,
//                                                         10, 20,
//                                                         10, 20);
            //cout<<"image"<<endl;
            V3DLONG mysz[4];
            mysz[0] = large.x-small.x;
            mysz[1] = large.y-small.y;
            mysz[2] = large.z-small.z;
            mysz[3] = in_zz[3];
            //cout<<"dim"<<mysz[0]<<" "<<mysz[1]<<" "<<mysz[2]<<" "<<mysz[3]<<endl;
            QString output_swc = output_dir+flag1+".eswc";
            //QString output_swc1=output_dir+"/";
            crop_swc_cuboid(nt, output_swc, crop_block);
            NeuronTree nt_crop_sorted;
            nt_crop_sorted=readSWC_file(output_swc);
            int branch_point=find_tip(nt_crop_sorted,mysz[0],mysz[1],mysz[2]);
            //cout <<"#################"<<branch_point<<endl;
            float bx = nt_crop_sorted.listNeuron.at(branch_point).x;
            float by = nt_crop_sorted.listNeuron.at(branch_point).y;
            float bz = nt_crop_sorted.listNeuron.at(branch_point).z;


            cout<<"check3"<<endl;

            unsigned char *data1d_crop=cropped_image;
            //distance
            int cchild1 = child1.at(branch[i]);
            int cchild2 = child2.at(branch[i]);

            //intensity
            //child1
            float cx1 = nt.listNeuron.at(alln.indexOf(cchild1)).x;
            //V3DLONG cy1 = nt.listNeuron.at(child1.at(branch[i])).y;
            float cy1 = nt.listNeuron.at(alln.indexOf(cchild1)).y;
            float cz1 = nt.listNeuron.at(alln.indexOf(cchild1)).z;

            //child2
            float cx2 = nt.listNeuron.at(alln.indexOf(cchild2)).x;
            float cy2 = nt.listNeuron.at(alln.indexOf(cchild2)).y;
            float cz2 = nt.listNeuron.at(alln.indexOf(cchild2)).z;


            //intensity
//            double avg1;
//            double avg2;
//            //avg1= average_intensity(data1d_crop_1,nt_sorted1,center1, nt_sorted1_size, diff1, mysz[0],mysz[1]);
//            avg1= average_intensity(data1d_crop_1,nt_sorted1,center1, nt_sorted1_size, diff1, 20,20);
//            avg2= average_intensity(data1d_crop_2,nt_sorted2,center2, nt_sorted2_size, diff2, 20,20);
//            cout<<avg1<<"~~~~~~~~~"<<avg2<<endl;
            cout<<"check4"<<endl;
            //angle: whether overlap or not
            int grandp;
            int bgp;
            bgp = plist.at(p_index);
            for (int k=0; k<5; k++){
                    grandp = alln.indexOf(bgp);
                    if(grandp>0){
                        bgp = plist.at(grandp);
                        continue;}
                    else{
                        break;
                    }
                }

            int branch_parent = grandp;
            float bpx = nt.listNeuron.at(branch_parent).x;
            float bpy = nt.listNeuron.at(branch_parent).y;
            float bpz = nt.listNeuron.at(branch_parent).z;
            XYZ BP = XYZ(bpx-nodex,bpy-nodey,bpz-nodez);


            block crop_block_p = offset_block(zcenter_block, XYZ(bpx,bpy,bpz));
            XYZ small_p = XYZ(crop_block_p.small);
            XYZ large_p=XYZ(crop_block_p.large);
            small_p.x = floor(small_p.x);
            small_p.y = floor(small_p.y);
            small_p.z = floor(small_p.z);
            large_p.x = ceil(large_p.x)+1;
            large_p.y = ceil(large_p.y)+1;
            large_p.z = ceil(large_p.z)+1;
            unsigned char * cropped_image_p = 0;
            //cout<<"dim"<<small.x<<small.y<<small.z<<large.x<<large.y<<large.z<<endl;
            cropped_image_p = callback.getSubVolumeTeraFly(image_file.toStdString(),
                                                         small_p.x, large_p.x,
                                                         small_p.y, large_p.y,
                                                         small_p.z, large_p.z);
            unsigned char *data1d_crop_p=cropped_image_p;



            QString out_swcp = output_dir+flag1+"_parent.eswc";
            crop_swc_cuboid(nt, out_swcp, crop_block_p);
            NeuronTree nt_sortedp;
            nt_sortedp=readSWC_file(out_swcp);
            int p_point=find_tip(nt_sortedp,mysz[0],mysz[1],mysz[2]);
            float px = nt_sortedp.listNeuron.at(p_point).x;
            float py = nt_sortedp.listNeuron.at(p_point).y;
            float pz = nt_sortedp.listNeuron.at(p_point).z;
            cout<<"check5"<<endl;

            //mask: cylinder
            unsigned char *mask =0;
            double margin=0;
            QList<int> marker_nodes;
            //cout<<nt_crop_sorted.listNeuron.size()<<endl;
            marker_nodes=find_tip_and_itspn(nt_crop_sorted,mysz[0],mysz[1],mysz[2]);
            //cout<<"marker_nodes "<<marker_nodes.size()<<endl;
            mask = new unsigned char [mysz[0]*mysz[1]*mysz[2]];
            memset(mask,0,mysz[0]*mysz[1]*mysz[2]*sizeof(unsigned char));
            Mask_filter(nt_crop_sorted, mask, mysz[0], mysz[1], mysz[2],marker_nodes,margin);
            V3DLONG stacksz =mysz[0]*mysz[1];
            unsigned char *image_mip=0;
            image_mip = new unsigned char [stacksz];
            unsigned char *label_mip=0;
            label_mip = new unsigned char [stacksz];
            cout<<"filter check"<<endl;
            for(V3DLONG iy = by-10; iy < bx+10; iy++)
            {
                V3DLONG offsetj = iy*mysz[0];
                for(V3DLONG ix = bx-10; ix < bx+10; ix++)
                {
                    int max_mip = 0;
                    int max_label = 0;
                    for(V3DLONG iz = bz-4; iz < bz+4; iz++)
                    {
                        V3DLONG offsetk = iz*mysz[1]*mysz[0];
                        if(data1d_crop[offsetk + offsetj + ix] >= max_mip)
                        {
                            image_mip[iz*mysz[1]*mysz[0]+iy*mysz[0] + ix] = data1d_crop[offsetk + offsetj + ix];
                            max_mip = data1d_crop[offsetk + offsetj + ix];
                        }
                        if(mask[offsetk + offsetj + ix] >= max_label)
                        {
                            label_mip[iz*mysz[1]*mysz[0]+iy*mysz[0] + ix] = mask[offsetk + offsetj + ix];
                            max_label = mask[offsetk + offsetj + ix];
                        }
                    }
                }
            }
            cout<<"mask check"<<endl;

//            double d1=dist(nt.listNeuron.at(p_index),nt.listNeuron.at(cchild1));
//            double d2=dist(nt.listNeuron.at(p_index),nt.listNeuron.at(cchild2));

            int child_11;
            int parent1 =cchild1;
            int p1;
            XYZ BC1;
            double ang1 = 0;
            double a1;
            p1 = cchild1;
            double d_11;
            for (int k=0; k<10; k++){
                if(plist.count(p1) <2){
                    child_11 = plist.indexOf(p1);                    
                    //cout<<"child_11````````````````"<<child_11<<endl;
                    if(child_11>0){
//                        d1=d1+dist(nt.listNeuron.at(p1),nt.listNeuron.at(child_11));
                        p1 = alln.at(child_11);
                        BC1 = XYZ(nt.listNeuron.at(child_11).x-nodex,nt.listNeuron.at(child_11).y-nodey,nt.listNeuron.at(child_11).z-nodez);
                        a1 = Angle(BP,BC1);
                        if(a1>ang1){
                            ang1 = a1;
                            parent1 = p1;
                        }
                        d_11=dist(nt.listNeuron.at(child_11),nt.listNeuron.at(p_index));
                        if(d_11>100|plist.at(child_11)==-1){break;}
                        continue;}
                    else if(child_11<0){
                        break;
                    }
                    }
                else{
//                   d1=d1+dist(nt.listNeuron.at(p1),nt.listNeuron.at(parent1));
                   int idx = alln.indexOf(parent1);
                   BC1 = XYZ(nt.listNeuron.at(idx).x-nodex,nt.listNeuron.at(idx).y-nodey,nt.listNeuron.at(idx).z-nodez);
                   ang1 = Angle(BP,BC1);
                   break;
                }
                }

            float ct1x = nt.listNeuron.at(alln.indexOf(parent1)).x;
            float ct1y = nt.listNeuron.at(alln.indexOf(parent1)).y;
            float ct1z = nt.listNeuron.at(alln.indexOf(parent1)).z;

            block crop_block_c1 = offset_block(zcenter_block, XYZ(ct1x,ct1y,ct1z));
            XYZ small_1 = XYZ(crop_block_c1.small);
            XYZ large_1=XYZ(crop_block_c1.large);
            small_1.x = floor(small_1.x);
            small_1.y = floor(small_1.y);
            small_1.z = floor(small_1.z);
            large_1.x = ceil(large_1.x)+1;
            large_1.y = ceil(large_1.y)+1;
            large_1.z = ceil(large_1.z)+1;
            unsigned char * cropped_image_1 = 0;
            //cout<<"dim"<<small.x<<small.y<<small.z<<large.x<<large.y<<large.z<<endl;
            cropped_image_1 = callback.getSubVolumeTeraFly(image_file.toStdString(),
                                                         small_1.x, large_1.x,
                                                         small_1.y, large_1.y,
                                                         small_1.z, large_1.z);
            unsigned char *data1d_crop_1=cropped_image_1;



            QString out_swc1 = output_dir+flag1+"_child1.eswc";
            crop_swc_cuboid(nt, out_swc1, crop_block_c1);
            NeuronTree nt_sorted1;
            nt_sorted1=readSWC_file(out_swc1);
            int c1_point=find_tip(nt_sorted1,mysz[0],mysz[1],mysz[2]);
            float childx1 = nt_sorted1.listNeuron.at(c1_point).x;
            float childy1 = nt_sorted1.listNeuron.at(c1_point).y;
            float childz1 = nt_sorted1.listNeuron.at(c1_point).z;
//            XYZ diff1 = XYZ((nodex-childx1)/2,(nodey-childy1)/2,(nodez-childz1)/2);
//            XYZ center1 = XYZ(childx1,childy1,childz1);
//            int nt_sorted1_size = nt_sorted1.listNeuron.size();




            int child_22;
            int parent2 =cchild2;
            int p2;
            double a2;
            double ang2=0;
            XYZ BC2;
            p2 = cchild2;
            double d_22;
            for (int k=0; k<10; k++){
                if(plist.count(p2) <2){
                child_22 = plist.indexOf(p2);
                if(child_22>=0){
//                    d2=d2+dist(nt.listNeuron.at(p2),nt.listNeuron.at(child_22));
                    p2 = alln.at(child_22);
                    //cout<<"```````````"<<idx<<endl;
                    BC2 = XYZ(nt.listNeuron.at(child_22).x-nodex,nt.listNeuron.at(child_22).y-nodey,nt.listNeuron.at(child_22).z-nodez);
                    a2 = Angle(BP,BC2);
                    if(a2>ang2){
                        ang2 = a2;
                        parent2 = p2;
                    }
                    d_22=dist(nt.listNeuron.at(child_22),nt.listNeuron.at(p_index));
                    if(d_22>100|plist.at(child_11)==-1){break;}
                    continue;}
                else if(child_22<0){
                    ang2 = a2;
                    break;
                }
                }
                else{
//                    d2=d2+dist(nt.listNeuron.at(p2),nt.listNeuron.at(parent2));
                    int idx = alln.indexOf(parent2);
                    BC2 = XYZ(nt.listNeuron.at(idx).x-nodex,nt.listNeuron.at(idx).y-nodey,nt.listNeuron.at(idx).z-nodez);
                    ang2 = Angle(BP,BC2);
                    break;
                }
            }

            float ct2x = nt.listNeuron.at(alln.indexOf(parent2)).x;
            float ct2y = nt.listNeuron.at(alln.indexOf(parent2)).y;
            float ct2z = nt.listNeuron.at(alln.indexOf(parent2)).z;

            block crop_block_c2 = offset_block(zcenter_block, XYZ(ct2x,ct2y,ct2z));
            XYZ small_2 = XYZ(crop_block_c2.small);
            XYZ large_2=XYZ(crop_block_c2.large);
            small_2.x = floor(small_2.x);
            small_2.y = floor(small_2.y);
            small_2.z = floor(small_2.z);
            large_2.x = ceil(large_2.x)+1;
            large_2.y = ceil(large_2.y)+1;
            large_2.z = ceil(large_2.z)+1;
            unsigned char * cropped_image_2 = 0;
            //cout<<"dim"<<small.x<<small.y<<small.z<<large.x<<large.y<<large.z<<endl;
            cropped_image_2 = callback.getSubVolumeTeraFly(image_file.toStdString(),
                                                         small_2.x, large_2.x,
                                                         small_2.y, large_2.y,
                                                         small_2.z, large_2.z);
            unsigned char *data1d_crop_2=cropped_image_2;
            QString out_swc2 = output_dir+flag1+"_child2.eswc";
            crop_swc_cuboid(nt, out_swc2, crop_block_c2);
            NeuronTree nt_sorted2;
            nt_sorted2=readSWC_file(out_swc2);
            int c2_point=find_tip(nt_sorted2,mysz[0],mysz[1],mysz[2]);
            float childx2 = nt_sorted2.listNeuron.at(c2_point).x;
            float childy2 = nt_sorted2.listNeuron.at(c2_point).y;
            float childz2 = nt_sorted2.listNeuron.at(c2_point).z;
//            XYZ diff2 = XYZ((nodex-childx2)/2,(nodey-childy2)/2,(nodez-childz2)/2);
//            XYZ center2 = XYZ(childx2,childy2,childz2);
//            int nt_sorted2_size=nt_sorted2.listNeuron.size();

            cout<<"check6"<<endl;

//            XYZ BCChild1=XYZ(cx1-nodex,cy1-nodey,cz1-nodez);
//            XYZ BCChild2=XYZ(cx2-nodex,cy2-nodey,cz2-nodez);
            int chd1,chd2;
            int part2 =cchild2;
            int part1 =cchild1;
            int pt2,pt1;
            double A;
            XYZ BCChild1=XYZ(ct1x-nodex,ct1y-nodey,ct1z-nodez);
            XYZ BCChild2=XYZ(ct2x-nodex,ct2y-nodey,ct2z-nodez);
            double ang3 = Angle(BCChild1,BCChild2);
            XYZ bc1,bc2;
            pt2 = cchild2;
            pt1=cchild1;

            for(int k=0;k<8;k++){
                if(plist.count(pt1)<2){
                    chd1=plist.indexOf(pt1);
                    if(chd1>=0){
                        pt1=alln.at(chd1);
                        bc1=XYZ(nt.listNeuron.at(chd1).x-nodex,nt.listNeuron.at(chd1).y-nodey,nt.listNeuron.at(chd1).z-nodez);
                        for(int m=0;m<8;m++){
                            if(plist.count(pt2)<2){
                                chd2=plist.indexOf(pt2);
                                if(chd2>=0){
                                    pt2=alln.at(chd2);
                                    bc2=XYZ(nt.listNeuron.at(chd2).x-nodex,nt.listNeuron.at(chd2).y-nodey,nt.listNeuron.at(chd2).z-nodez);
                                    A=Angle(bc1,bc2);
                                    if(A>ang3){
                                        ang3=A;
                                        part1=pt1;
                                        part2=pt2;
                                    }
                                    continue;
                                }
                                else{
                                    if(A>ang3){ang3=A;}
                                    break;
                                }
                            }
                            else{
                                int id = alln.indexOf(part2);
                                bc2 = XYZ(nt.listNeuron.at(id).x-nodex,nt.listNeuron.at(id).y-nodey,nt.listNeuron.at(id).z-nodez);
                                double g = Angle(bc1,bc2);
                                if(g>ang3){ang3=g;}
                                break;
                            }
                        }
                    }
                    else{
                        if(A>ang3){ang3=A;}
                       break;
                    }
                }
                else{
                    int id = alln.indexOf(part1);
                    bc1 = XYZ(nt.listNeuron.at(id).x-nodex,nt.listNeuron.at(id).y-nodey,nt.listNeuron.at(id).z-nodez);
                    for(int m=0;m<8;m++){
                        if(plist.count(pt2)<2){
                            chd2=plist.indexOf(pt2);
                            if(chd2>=0){
                                pt2=alln.at(chd2);
                                bc2=XYZ(nt.listNeuron.at(chd2).x-nodex,nt.listNeuron.at(chd2).y-nodey,nt.listNeuron.at(chd2).z-nodez);
                                A=Angle(bc1,bc2);
                                if(A>ang3){
                                    ang3=A;
                                    part1=pt1;
                                    part2=pt2;
                                }
                                continue;
                            }
                            else{
                                if(A>ang3){ang3=A;}
                                break;
                            }
                        }
                        else{
                            int id = alln.indexOf(part2);
                            bc2 = XYZ(nt.listNeuron.at(id).x-nodex,nt.listNeuron.at(id).y-nodey,nt.listNeuron.at(id).z-nodez);
                            double g = Angle(bc1,bc2);
                            if(g>ang3){ang3=g;}
                            break;
                        }
                    }
                    break;
                }
            }
            cout<<"check7"<<endl;



            double d1,d2;
            d1=dist(nt.listNeuron.at(p_index),nt.listNeuron.at(alln.indexOf(parent1)));
            d2=dist(nt.listNeuron.at(p_index),nt.listNeuron.at(alln.indexOf(parent2)));

//            int grandch1 = plist.indexOf(cchild1);
//            int grandch2 = plist.indexOf(cchild2);
//            cout<<grandch1<<" "<<grandch2<<endl;
//            if(grandch1 <0){
//                d1 = dist(nt.listNeuron.at(cchild1), nt.listNeuron.at(p_index));
//            //cout<<"cchild1 distance:"<<d1<<endl;
//            }
//            else{d1 = 50;}
//            if(grandch2 <0){
//                d2 = dist(nt.listNeuron.at(cchild2), nt.listNeuron.at(p_index));
//            //cout<<"cchild2 distance:"<<d2<<endl;
//            }
//            else{d2 = 50;}
            //cout<<branch.at(i)<<"............."<<endl;
            cout<<"check8"<<endl;

//            vector<MyMarker> allmarkers1;  // allmarkers: pixels within the cylinder specified by n1&n2;
//            vector<MyMarker> allmarkers2;
//            MyMarker m = MyMarker(bpx, bpy,bpz);
//            MyMarker m1 = MyMarker(cx1,cy1,cz1);
//            MyMarker m2 = MyMarker(cx2,cy2,cz2);
//            m.radius = SWC_RADIUS;
//            m1.radius = SWC_RADIUS;
//            m2.radius = SWC_RADIUS;
//            getMarkersBetween(allmarkers1, m, m1);
//            double sig1;
//            sig1= signal_at_markers(allmarkers1, data1d_crop, mysz[0], mysz[1], mysz[2]);
//            getMarkersBetween(allmarkers2, m, m2);
//            double sig2;
//            sig2 = signal_at_markers(allmarkers2, data1d_crop, mysz[0], mysz[1], mysz[2]);
            //cout<<allmarkers1.size()<<" "<<allmarkers2.size()<<endl;
            //cout<<sig1<<"**********************"<<sig2<<"***********signal"<<endl;
            double imgave,imgstd;
            V3DLONG total_size=4*125;
            mean_and_std(data1d_crop,total_size,imgave,imgstd);
            double img_threshold;
            if(imgstd>10){
                img_threshold = imgave*0.5-imgstd*0.3;
            }
            else{img_threshold=imgave*0.3;}
//            double td= (imgstd>10) ? 10:imgstd;
//            double img_threshold= imgave+0.7*td+30;
//            double img_threshold= imgave*0.5+td*0.05;
            //cout<<"***********"<<img_threshold<<endl;
            double imgave1,imgstd1;
            mean_and_std(data1d_crop_1,total_size,imgave1,imgstd1);
            double thrd1;
            if(imgstd1>10){
                thrd1 = imgave1*0.7-imgstd1*0.3;
            }
            else{thrd1=imgave1*0.6;}
//            double td1= (imgstd1>10) ? 10:imgstd1;
//            double thrd1= imgave1*0.5+td1*0.05;
            double imgave2,imgstd2;
            mean_and_std(data1d_crop_2,total_size,imgave2,imgstd2);
            double thrd2;
            if(imgstd2>10){
                thrd2 = imgave2*0.7-imgstd2*0.3;
            }
            else{thrd2=imgave2*0.6;}
//            double td2= (imgstd2>10) ? 10:imgstd2;
//            double thrd2= imgave2*0.5+td2*0.05;
            cout<<"check9"<<endl;

            if((data1d_crop[V3DLONG(bz*mysz[0]*mysz[1]+by*mysz[0]+bx)]>img_threshold)&(data1d_crop_p[V3DLONG(pz*mysz[0]*mysz[1]+py*mysz[0]+px)]>img_threshold)){
                 if(((data1d_crop_1[V3DLONG(childz1*mysz[0]*mysz[1]+childy1*mysz[0]+childx1)]>thrd1)&(data1d_crop_2[V3DLONG(childz2*mysz[0]*mysz[1]+childy2*mysz[0]+childx2)]>thrd2))|((d1>5)&(d2>5))){
                     if((ang1>7)&(ang2>7)&(ang3>7)){
//                      if((d1>5)&(d2>5)){
                        branch_list.push_back(p_index);
                        NeuronSWC cur = nt.listNeuron.at(p_index);
                        apo_branch.push_back(cur);
                        //cout<<ang1<<"++++++++++++"<<ang2<<"+++++++++"<<ang2<<endl;
//                        local_angle +=ang3;
//                        count +=1;
//                        ofstream write;
//                        write.open("/home/penglab/Desktop/bd.txt",ios::app);
//                        write<<cur.n<<" "<<cchild1<<" "<<cchild2<<" "<<ang3<<endl;
//                        write<<nodex<<" "<<nodey<<" "<<nodez<<endl;
//                        write<<cx1<<" "<<cy1<<" "<<cz1<<" "<<cx2<<" "<<cy2<<" "<<cz2<<endl;
//                        write.close();
//                }
//                else{
//                    NeuronSWC cur = nt.listNeuron.at(branch[i]-1);
//                    apo_dis.push_back(cur);
//                    //cout<<"intensity!!!!!!!!!"<<endl;
//                }
            }
            else{
                NeuronSWC cur = nt.listNeuron.at(p_index);
                apo_angle.push_back(cur);
                //cout<<d1<<" "<<d2<<"^^^^^^^^^^^^^"<<endl;
//                ofstream write;
//                write.open("/home/penglab/Desktop/dis.txt",ios::app);
//                write<<cur.n<<" "<<d1<<" "<<d2<<endl;
//                write.close();
            }
            }
                else{
                    NeuronSWC cur = nt.listNeuron.at(p_index);
                    apo_dis.push_back(cur);
                }
            }
            else{
                NeuronSWC cur = nt.listNeuron.at(p_index);
                apo_list.push_back(cur);
            }
            cout<<"check55"<<endl;
    //}
            }
    }
//    double average_local_angle = local_angle/count;

//    cout<<average_local_angle<<endl;
//    ofstream write;
//    write.open("/home/penglab/Desktop/avg_angle.txt",ios::app);
//    write<<average_local_angle<<endl;
//    write.close();


    cout<<branch_list.size()<<"............."<<apo_list.size()<<endl;
    //create .apo
    unsigned int Vsize=50;
    QList<CellAPO> apo;

    for(int i = 0; i <apo_list.size();i++)
    {
        CellAPO m;
        m.x = apo_list.at(i).x;
        m.y = apo_list.at(i).y;
        m.z = apo_list.at(i).z;
        m.color.r=255;
        m.color.g=0;
        m.color.b=0;
        m.volsize = Vsize;
        apo.push_back(m);
    }
    for(int i = 0; i <apo_dis.size();i++)
    {
        CellAPO m;
        m.x = apo_dis.at(i).x;
        m.y = apo_dis.at(i).y;
        m.z = apo_dis.at(i).z;
        m.color.r=255;
        m.color.g=0;
        m.color.b=255;
        m.volsize = Vsize;
        apo.push_back(m);
    }
    for(int i = 0; i <apo_angle.size();i++)
    {
        CellAPO m;
        m.x = apo_angle.at(i).x;
        m.y = apo_angle.at(i).y;
        m.z = apo_angle.at(i).z;
        m.color.r=255;
        m.color.g=255;
        m.color.b=0;
        m.volsize = Vsize;
        apo.push_back(m);
    }
    for(int i = 0; i <apo_branch.size();i++)
    {
        CellAPO b;
        b.x = apo_branch.at(i).x;
        b.y = apo_branch.at(i).y;
        b.z = apo_branch.at(i).z;
        b.color.r=0;
        b.color.g=255;
        b.color.b=0;
        b.volsize = Vsize;
        apo.push_back(b);
    }

    //QString apo_name = filename + ".apo";

    writeAPO_file(output_branch,apo);

    //cout<<"number of branch points "<<branch_list.size()<<endl;


    //QList<int> branch_list = get_branch_points(callback, nt, false, image_file,output_apo);
//    cout<<"Number_of_tips\t"<<qPrintable(swc_file)<<"\t"<<tip_list.size()<<endl;
    //    // Crop tip-centered regions one by one4
    //    block zcenter_block; // This is a block centered at (0,0,0)
    //    zcenter_block.small = 0-block_size/2;
    //    zcenter_block.large = block_size/2;
    //    QList<QString> output_suffix;
    //    output_suffix.append(QString("nrrd"));
    //    output_suffix.append(QString("swc"));

    //    for(int i=0; i<tip_list.size(); i++){
    //       if(i>0){break;}
    //        NeuronSWC node = nt.listNeuron.at(tip_list.at(i));
    //        qDebug()<<node.n;
    //        if(node.type > 5){continue;}
    //        // create a tip-centered block
    //        XYZ shift;
    //        shift.x = (int)node.x;
    //        shift.y = (int)node.y;
    //        shift.z = (int)node.z;

    //        block crop_block = offset_block(zcenter_block, shift);
    //        crop_block.name = cell_name + "_"+QString::number(i);
    //        // crop image
    //        qDebug()<<crop_block.name;
    //        qDebug()<<crop_block.small.x<<crop_block.small.y<<crop_block.small.z;
    //        qDebug()<<crop_block.large.x<<crop_block.large.y<<crop_block.large.z;

    //        crop_img(image_file, crop_block, output_dir, callback, QString(".nrrd"));
    //        // crop swc
    //        QString output_swc = output_dir+crop_block.name+".swc";
    //        crop_swc(swc_file, output_swc, crop_block);
    //        my_saveANO(output_dir, crop_block.name, output_suffix);
    //    }
      return;
}



double Angle(XYZ p1,XYZ p2){
    double dot = p1.x*p2.x+p1.y*p2.y+p1.z*p2.z;
    double norm1 = sqrt(p1.x*p1.x+p1.y*p1.y+p1.z*p1.z);
    double norm2 = sqrt(p2.x*p2.x+p2.y*p2.y+p2.z*p2.z);
    double cross = norm1*norm2;
    if(cross == 0){
        return 0;
        //cout<<"$$$$$$$$$$$$$$$"<<endl;
    }
    else{
        double theta = acos(dot/(cross));
        theta = theta*180.0/PI;
        return theta;
    }

}

QList<int> find_tip_and_itspn(NeuronTree nt, long sz0, long sz1, long sz2)//for mark 2D image witg different coulor indifferent part
{
    // Return the node at center of the image as tip node
    QList<int> nodes_to_be_marked;
    MyMarker center = MyMarker((sz0-1)/2, (sz1-1)/2, (sz2-1)/2);
    QVector<QVector<V3DLONG> > childs;
    V3DLONG neuronNum = nt.listNeuron.size();
    childs = QVector< QVector<V3DLONG> >(neuronNum, QVector<V3DLONG>() );
    //cout<<neuronNum<<endl;
    for (V3DLONG i=0;i<neuronNum;i++)
    {
        V3DLONG par = nt.listNeuron[i].pn;
        if (par<0) continue;
        childs[nt.hashNeuron.value(par)].push_back(i);

    }
    //cout<<childs.size()<<endl;
    QList<int> plist;
    QList<int> alln;
    int N=nt.listNeuron.size();
    for(int i=0; i<N; i++){
        plist.append(nt.listNeuron.at(i).pn);
        alln.append(nt.listNeuron.at(i).n);
    }
    //cout<<"2************"<<endl;
    for(int i=0; i<nt.listNeuron.size(); i++){
        MyMarker node=MyMarker(nt.listNeuron.at(i).x, nt.listNeuron.at(i).y, nt.listNeuron.at(i).z);
        if(marker_dist(center, node , false)<MIN_DIST & childs[i].size()==2) {
            nodes_to_be_marked.push_back(i);
        }
        else if(marker_dist(center, node, false)<MIN_DIST+1 & childs[i].size()==2) {
            nodes_to_be_marked.push_back(i);
        }
        else if(marker_dist(center, node, false)<MIN_DIST+2 & childs[i].size()==2) {
            nodes_to_be_marked.push_back(i);
        }
    }

    //cout<<nodes_to_be_marked.size()<<endl;
    if(nodes_to_be_marked.size()==1){

        int index_pn=alln.indexOf(nt.listNeuron.at(nodes_to_be_marked.at(0)).pn);
        nodes_to_be_marked.push_back(index_pn);
    }

    return nodes_to_be_marked;
}

QHash<V3DLONG, V3DLONG> NeuronNextPn(const NeuronTree &neurons)
{
    QHash<V3DLONG, V3DLONG> neuron_id_table;
    for (V3DLONG i=0;i<neurons.listNeuron.size(); i++)
        neuron_id_table.insert(V3DLONG(neurons.listNeuron.at(i).n), i);
    return neuron_id_table;
}

void Mask_filter(NeuronTree neurons,unsigned char*Mask, V3DLONG mysz0,V3DLONG mysz1,V3DLONG mysz2,QList<int> marker_node,double margin){
    NeuronSWC *p_cur = 0;
    //create a LUT
    QHash<V3DLONG, V3DLONG> neuron_id_table = NeuronNextPn(neurons);
    //compute mask
    double xs = 0, ys = 0, zs = 0, xe = 0, ye = 0, ze = 0, rs = 0, re = 0;
    V3DLONG pagesz = mysz0*mysz1;
    //cout<<"1**********"<<endl;
    for (V3DLONG ii=0; ii<neurons.listNeuron.size(); ii++)
    {
        V3DLONG i,j,k;
        p_cur = (NeuronSWC *)(&(neurons.listNeuron.at(ii)));
        xs = p_cur->x;
        ys = p_cur->y;
        zs = p_cur->z;
        if(xs<0 || ys<0 || zs<0)
            continue;
        rs = p_cur->r+margin;//margin added by PHC 20170531
        double ballx0, ballx1, bally0, bally1, ballz0, ballz1, tmpf;

        ballx0 = xs - rs; ballx0 = qBound(double(0), ballx0, double(mysz0-1));
        ballx1 = xs + rs; ballx1 = qBound(double(0), ballx1, double(mysz0-1));
        if (ballx0>ballx1) {tmpf = ballx0; ballx0 = ballx1; ballx1 = tmpf;}

        bally0 = ys - rs; bally0 = qBound(double(0), bally0, double(mysz1-1));
        bally1 = ys + rs; bally1 = qBound(double(0), bally1, double(mysz1-1));
        if (bally0>bally1) {tmpf = bally0; bally0 = bally1; bally1 = tmpf;}

        ballz0 = zs - rs; ballz0 = qBound(double(0), ballz0, double(mysz2-1));
        ballz1 = zs + rs; ballz1 = qBound(double(0), ballz1, double(mysz2-1));
        if (ballz0>ballz1) {tmpf = ballz0; ballz0 = ballz1; ballz1 = tmpf;}
        //cout<<"1**********"<<endl;
        if(ii==marker_node.at(0)){ //in this case only mark the first node(tip node),added by OYQ 2019.3.23.
        for (k = ballz0; k <= ballz1; k++){
            for (j = bally0; j <= bally1; j++){
                for (i = ballx0; i <= ballx1; i++){
                    V3DLONG ind = (k)*pagesz + (j)*mysz0 + i;
                    if (Mask[ind]>0) continue;
                    double norms10 = (xs-i)*(xs-i) + (ys-j)*(ys-j) + (zs-k)*(zs-k);
                    double dt = sqrt(norms10);
                    if(dt <=rs || dt<=1) Mask[ind] = 254;
                }
            }
          }
        cout<<"2**********"<<endl;

        if (p_cur->pn < 0) continue;//then it is root node already
        //get the parent info
        const NeuronSWC & pp  = neurons.listNeuron.at(neuron_id_table.value(p_cur->pn));
        xe = pp.x;
        ye = pp.y;
        ze = pp.z;
        re = pp.r;

        //judge if two points overlap, if yes, then do nothing as the sphere has already been drawn
        if (xe==xs && ye==ys && ze==zs)
        {
            v3d_msg(QString("Detect overlapping coordinates of node\n"), 0);
            continue;
        }

        double l =sqrt((xe-xs)*(xe-xs)+(ye-ys)*(ye-ys)+(ze-zs)*(ze-zs));
        double dx = (xe - xs);
        double dy = (ye - ys);
        double dz = (ze - zs);
        double x = xs;
        double y = ys;
        double z = zs;

        int steps = lroundf(l);
        steps = (steps < fabs(dx))? fabs(dx):steps;
        steps = (steps < fabs(dy))? fabs(dy):steps;
        steps = (steps < fabs(dz))? fabs(dz):steps;
        if (steps<1) steps =1;

        double xIncrement = double(dx) / (steps*2);
        double yIncrement = double(dy) / (steps*2);
        double zIncrement = double(dz) / (steps*2);

        V3DLONG idex1=lroundf(z)*mysz0*mysz1 + lroundf(y)*mysz0 + lroundf(x);
        if (lroundf(z)>(mysz2-1)||lroundf(y)>(mysz1-1)||lroundf(x)>(mysz0-1)) continue;
         Mask[idex1] = 254;

         for (int i = 0; i <= steps; i++)
         {
             x += xIncrement;
             y += yIncrement;
             z += zIncrement;

             x = ( x > mysz0 )? mysz0 : x;
             y = ( y > mysz1 )? mysz1 : y;
             z = ( z > mysz2 )? mysz2 : z;

             V3DLONG idex=lroundf(z)*mysz0*mysz1 + lroundf(y)*mysz0 + lroundf(x);
             if (Mask[idex]>0) continue;
             if (lroundf(z)>(mysz2-1)||lroundf(y)>(mysz1-1)||lroundf(x)>(mysz0-1)) continue;
             Mask[idex] = 254;
         }

         double rbox = (rs>re) ? rs : re;
         double x_down = (xs < xe) ? xs : xe; x_down -= rbox; x_down = V3DLONG(x_down); if (x_down<0) x_down=0; if (x_down>=mysz0-1) x_down = mysz0-1;
         double x_top  = (xs > xe) ? xs : xe; x_top  += rbox; x_top  = V3DLONG(x_top ); if (x_top<0)  x_top=0;  if (x_top>=mysz0-1)  x_top  = mysz0-1;
         double y_down = (ys < ye) ? ys : ye; y_down -= rbox; y_down = V3DLONG(y_down); if (y_down<0) y_down=0; if (y_down>=mysz1-1) y_down = mysz1-1;
         double y_top  = (ys > ye) ? ys : ye; y_top  += rbox; y_top  = V3DLONG(y_top ); if (y_top<0)  y_top=0;  if (y_top>=mysz1-1)  y_top = mysz1-1;
         double z_down = (zs < ze) ? zs : ze; z_down -= rbox; z_down = V3DLONG(z_down); if (z_down<0) z_down=0; if (z_down>=mysz2-1) z_down = mysz2-1;
         double z_top  = (zs > ze) ? zs : ze; z_top  += rbox; z_top  = V3DLONG(z_top ); if (z_top<0)  z_top=0;  if (z_top>=mysz2-1)  z_top = mysz2-1;

         //compute cylinder and flag mask

         for (k=z_down; k<=z_top; k++)
         {
             for (j=y_down; j<=y_top; j++)
             {
                 for (i=x_down; i<=x_top; i++)
                 {
                     double rr = 0;
                     double countxsi = (xs-i);
                     double countysj = (ys-j);
                     double countzsk = (zs-k);
                     double countxes = (xe-xs);
                     double countyes = (ye-ys);
                     double countzes = (ze-zs);
                     double norms10 = countxsi * countxsi + countysj * countysj + countzsk * countzsk;
                     double norms21 = countxes * countxes + countyes * countyes + countzes * countzes;
                     double dots1021 = countxsi * countxes + countysj * countyes + countzsk * countzes;
                     double dist = sqrt( norms10 - (dots1021*dots1021)/(norms21) );
                     double t1 = -dots1021/norms21;
                     if(t1<0) dist = sqrt(norms10);
                     else if(t1>1)
                         dist = sqrt((xe-i)*(xe-i) + (ye-j)*(ye-j) + (ze-k)*(ze-k));
                     //compute rr
                     if (rs==re) rr =rs;
                     else
                     {
                         // compute point of intersection
                         double v1 = xe - xs;
                         double v2 = ye - ys;
                         double v3 = ze - zs;
                         double vpt = v1*v1 + v2*v2 +v3*v3;
                         double t = (double(i-xs)*v1 +double(j-ys)*v2 + double(k-zs)*v3)/vpt;
                         double xc = xs + v1*t;
                         double yc = ys + v2*t;
                         double zc = zs + v3*t;
                         double normssc = sqrt((xs-xc)*(xs-xc)+(ys-yc)*(ys-yc)+(zs-zc)*(zs-zc));
                         double normsce = sqrt((xe-xc)*(xe-xc)+(ye-yc)*(ye-yc)+(ze-zc)*(ze-zc));
                         rr = (rs >= re) ? (rs - ((rs - re)/sqrt(norms21))*normssc) : (re - ((re-rs)/sqrt(norms21))*normsce);
                     }
                     V3DLONG ind1 = (k)*mysz0*mysz1 + (j)*mysz0 + i;
                     if (Mask[ind1]>0) continue;
                     if (lroundf(z)>(mysz2-1)||lroundf(y)>(mysz1-1)||lroundf(x)>(mysz0-1)) continue;
                     if (dist <= rr || dist<=1)
                     {
                         Mask[ind1] = 254;
                     }
                 }
             }
         }

        }
    }
}



//double average_intensity(unsigned char *data1d_crop,NeuronTree nt, XYZ center, int size, XYZ diff, long mysz0,long mysz1){
//    XYZ m = XYZ(fabs(diff.x),fabs(diff.y),fabs(diff.z));
//    double intensity=0;
//    int count=0;
//    for(int i=0;i<size;i++){
//        if((fabs(nt.listNeuron.at(i).x-center.x)<m.x)&(fabs(nt.listNeuron.at(i).y-center.y)<m.y)&(fabs(nt.listNeuron.at(i).z-center.z)<m.z)){
//            intensity = intensity+data1d_crop[V3DLONG(nt.listNeuron.at(i).z*mysz0*mysz1+nt.listNeuron.at(i).y*mysz0+nt.listNeuron.at(i).z)];
//            count++;
//        }
//    }
//    double avg_intensity;
//    if(count==0){
//        avg_intensity =0;
//    }
//    else{
//        avg_intensity = intensity/count;
//    }
//    return avg_intensity;
//}
MyMarker adjust_size(MyMarker marker, double ratio_x=RX, double ratio_y=RY, double ratio_z=RZ){
    marker.x *= ratio_x;
    marker.y *= ratio_y;
    marker.z *= ratio_z;
    return marker;
}

MyMarker readjust_size(MyMarker marker, double ratio_x=RX, double ratio_y=RY, double ratio_z=RZ){
    marker.x /= ratio_x;
    marker.y /= ratio_y;
    marker.z /= ratio_z;
    return marker;
}

bool getMarkersBetween(vector<MyMarker> &allmarkers, MyMarker m1, MyMarker m2)
{
//    printf("welcome to use getMarkersBetween\n");
//    qDebug()<<m1.x<<m1.y<<m1.z;
//    qDebug()<<m2.x<<m2.y<<m2.z;

    double ratio_x = 1;
    double ratio_y = 1;
    double ratio_z = 5;
    m1 = adjust_size(m1, ratio_x, ratio_y, ratio_z);
    m2 = adjust_size(m2, ratio_x, ratio_y, ratio_z);

    double A = m2.x - m1.x;
    double B = m2.y - m1.y;
    double C = m2.z - m1.z;
    double R = m2.radius - m1.radius;
    double D = sqrt(A*A + B*B + C*C);
    A = A/D; B = B/D; C = C/D; R = R/D;

//    double ctz = A/sqrt(A*A + B*B);
//    double stz = B/sqrt(A*A + B*B);

//    double cty = C/sqrt(A*A + B*B + C*C);
//    double sty = sqrt(A*A + B*B)/sqrt(A*A + B*B + C*C);

    double x0 = m1.x;
    double y0 = m1.y;
    double z0 = m1.z;
    double r0 = m1.radius;

    set<MyMarker> marker_set;

    for(double t = 0.0; t <= marker_dist(m1, m2, false); t += 1.0)
    {
        MyMarker marker;
        int cx = x0 + A*t + 0.5;
        int cy = y0 + B*t + 0.5;
        int cz = z0 + C*t + 0.5;
        int radius = r0 + R*t + 0.5;
        int radius2 = radius * radius;

        for(int k = -radius; k <= radius; k++)
        {
            for(int j = -radius; j <= radius; j++)
            {
                for(int i = -radius; i <= radius; i++)
                {
                    if(i * i + j * j + k * k > radius2) continue;
                    double x = i, y = j, z = k;
//                    double x1, y1, z1;
//                    y1 = y * ctz - x * stz; x1 = x * ctz + y * stz; y = y1; x = x1;
//                    x1 = x * cty + z * sty; z1 = z * cty - x * sty; x = x1; z = z1;
//                    z1 = z * ctz + y * stz; y1 = y * ctz - z * stz; z = z1; y = y1;
                    x += cx; y += cy; z += cz;
                    marker = MyMarker(x, y, z);
                    marker = readjust_size(marker, ratio_x, ratio_y, ratio_z);
                    marker.x = (int)(marker.x+0.5);
                    marker.y= (int)(marker.y+0.5);
                    marker.z = (int)(marker.z+0.5);
                    marker_set.insert(marker);
                }
            }
        }
    }

    allmarkers.insert(allmarkers.end(), marker_set.begin(), marker_set.end());
//    qDebug()<<"Done getting markers";
    return true;
}

double signal_at_markers(vector<MyMarker> allmarkers, unsigned char * data1d, long sz0, long sz1, long sz2)
{
//    printf("welcome to use signal_at_markers\n");
    double signal = 0;
    double in_block_ct = 0;
    long sz01 = sz0 * sz1;
    long total_sz = sz0 * sz1 * sz2;
    unsigned char * outimg1d=0;


    for(int i=0; i<allmarkers.size();i++){
        int x = allmarkers[i].x;
        int y = allmarkers[i].y;
        int z = allmarkers[i].z;
        if((x<0) || (x>=sz0) || (y<0) || (y>=sz1) || (z<0) || (z>=sz2)){continue;} // out of image border
        int id = z*sz01+y*sz0+x;
//        cout<<id<<"\t";
        if(id<total_sz){
            signal += data1d[id];
            in_block_ct += 1;
            cout<<"******************************"<<signal<<endl;
        }
//        cout<<"pass"<<endl;
    }
//    qDebug()<<"Done collecting pixels";
    if(in_block_ct>0)
    {
        signal = signal / in_block_ct;
    }
    return signal;
}

bool  crop_swc_cuboid(NeuronTree nt, QString qs_output,block input_block)
{
    double xs=input_block.small.x; double ys=input_block.small.y; double zs=input_block.small.z;
    double xe=input_block.large.x; double ye=input_block.large.y; double ze=input_block.large.z;
    double xshift=input_block.small.x;double yshift=input_block.small.y; double zshift=input_block.small.z;
    printf("welcome to use crop_swc_cuboid\n");
    // 1. read input
    XYZ small(xs, ys, zs);
    XYZ large(xe, ye, ze);
    qDebug()<<small.x<<small.y<<small.z;
    qDebug()<<large.x<<large.y<<large.z;
    //NeuronTree nt = readSWC_file(qs_input);
    // 2. Decide which nodes are within the cuboid
    QList <int> inside_nlist;
    for(int i=0; i<nt.listNeuron.size(); i++){
        NeuronSWC node = nt.listNeuron.at(i);
        if(in_cuboid(node, small, large)){inside_nlist.append(node.n);}
    }
    qDebug()<<inside_nlist.size();
    //3. create a cropped tree
    NeuronTree new_tree;
    for(int i=0; i<nt.listNeuron.size(); i++){
        NeuronSWC node = nt.listNeuron.at(i);
        // If node is outside of the shell, skip to the next node.
        if(inside_nlist.lastIndexOf(node.n)<0){
            continue;
        }
        // If the parent node is to be deleted, put itself as a parent node.
        if(inside_nlist.lastIndexOf(node.pn)<0){
            node.pn = -1;
        }
        new_tree.listNeuron.append(node);
    }
    //NeuronSWC noderoot = nt.listNeuron.at(i);
    //new_tree.listNeuron.at(0).pn=-1;
    nt.deepCopy(my_SortSWC(new_tree, VOID, 0));

    //4. shift if needed
    if((xshift!=0) || (yshift!=0) || (zshift!=0)){
        for(int i=0; i<nt.listNeuron.size(); i++){
            nt.listNeuron[i].x -= xshift;
            nt.listNeuron[i].y -= yshift;
            nt.listNeuron[i].z -= zshift;
        }
    }
    // 5. save output
    export_list22file(nt.listNeuron, qs_output);
    return 1;
}
//NeuronTree my_SortSWC(NeuronTree nt, V3DLONG newrootid, double thres){
//    NeuronTree new_tree;
//    QList<NeuronSWC> neuronlist;
////    v3d_msg(QString("id is %1, ths is %2").arg(newrootid).arg(thres));
////    writeSWC_file("C:/Users/pengx/Desktop/test/tmp.swc",nt);
//    SortSWC(nt.listNeuron, neuronlist, newrootid, thres);
////    v3d_msg("done!");
//    new_tree.deepCopy(neuronlist_2_neurontree(neuronlist));
//    return new_tree;
//}
NeuronTree neuronlist_2_neurontree(QList<NeuronSWC> neuronlist){
    NeuronTree new_tree;
    QList<NeuronSWC> listNeuron;
    QHash <int, int>  hashNeuron;
    listNeuron.clear();
    hashNeuron.clear();
//    qDebug()<<"creating new neuronlist";
    for (int i = 0; i < neuronlist.size(); i++)
    {
        NeuronSWC node=neuronlist.at(i);
        NeuronSWC S;
        S.n 	= node.n;
        S.type 	= node.type;
        S.x 	= node.x;
        S.y 	= node.y;
        S.z 	= node.z;
        S.r 	= node.r;
        S.pn 	= node.pn;
        listNeuron.append(S);
        hashNeuron.insert(S.n, i);
    }

    new_tree.listNeuron = listNeuron;
    new_tree.hashNeuron = hashNeuron;
    return new_tree;
}
bool in_cuboid(NeuronSWC node, XYZ small, XYZ large){
    if((node.x>=small.x) & (node.x<=large.x) &
            (node.y>=small.y) & (node.y<=large.y) &
            (node.z>=small.z) & (node.z<=large.z)
            )
    {
        return 1;
    }
    return 0;
}
bool export_list22file(const QList<NeuronSWC>& lN, QString fileSaveName)
{
    QFile file(fileSaveName);
    if (!file.open(QIODevice::WriteOnly|QIODevice::Text))
        return false;
    bool eswc_flag=false;
    if(fileSaveName.section('.',-1).toUpper()=="ESWC")
        eswc_flag=true;
    QTextStream myfile(&file);
    myfile<<"# generated by Vaa3D Plugin neuron_connector"<<endl;
    if(eswc_flag)
        myfile<<"##n,type,x,y,z,radius,parent,segment_id,segment_layer,feature_value"<<endl;
    else
        myfile<<"##n,type,x,y,z,radius,parent"<<endl;
    for (V3DLONG i=0;i<lN.size();i++){
        myfile << lN.at(i).n <<" " << lN.at(i).type << " "<< lN.at(i).x <<" "<<lN.at(i).y << " "<< lN.at(i).z << " "<< lN.at(i).r << " " <<lN.at(i).pn;
        if(eswc_flag){
            myfile<<" "<<lN.at(i).seg_id<<" "<<lN.at(i).level;
            for(int j=0; j<lN.at(i).fea_val.size(); j++)
                myfile <<" "<< lN.at(i).fea_val.at(j);
        }
        myfile << endl;
    }
    file.close();
    return true;
}
int find_tip(NeuronTree nt, long sz0, long sz1, long sz2)
{
    // Return the node at center of the image as tip node
    MyMarker center = MyMarker((sz0-1)/2, (sz1-1)/2, (sz2-1)/2);
    QVector<QVector<V3DLONG> > childs;
    V3DLONG neuronNum = nt.listNeuron.size();
    childs = QVector< QVector<V3DLONG> >(neuronNum, QVector<V3DLONG>() );
    for (V3DLONG i=0;i<neuronNum;i++)
    {
        V3DLONG par = nt.listNeuron[i].pn;
        if (par<0) continue;
        childs[nt.hashNeuron.value(par)].push_back(i);
    }

    for(int i=0; i<nt.listNeuron.size(); i++){
        MyMarker node=MyMarker(nt.listNeuron.at(i).x, nt.listNeuron.at(i).y, nt.listNeuron.at(i).z);
        if(marker_dist(center, node,true)<MIN_DIST ) {
            return i;
        }
        else if(marker_dist(center, node,true)<MIN_DIST+1 ) {
            return i;
        }
        else if(marker_dist(center, node,true)<MIN_DIST+2 ) {
            return i;
        }
    }
    printf("No tip found!\n");
}

block offset_block(block input_block, XYZ offset)
{
    input_block.small = offset_XYZ(input_block.small, offset);
    input_block.large = offset_XYZ(input_block.large, offset);
    return input_block;
}



XYZ offset_XYZ(XYZ input, XYZ offset){
    input.x += offset.x;
    input.y += offset.y;
    input.z += offset.z;
    return input;
}

//void crop_swc(QString input_swc, QString output_swc, block crop_block)
//{

////        crop_img(image_file, crop_block, output_dir, callback, QString(".nrrd"));
////        // crop swc
////        QString output_swc = output_dir+crop_block.name+".swc";
////        crop_swc(swc_file, output_swc, crop_block);
//////        my_saveANO(output_dir, crop_block.name, output_suffix);
////    }
//    return;
//}

//void get2d_image(const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback)
//{
//    vector<char*> infiles, inparas, outfiles;
//    if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
//    if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
//    if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);
//    QString input_swc=infiles.at(0);
//    QString input_image=inparas.at(0);
//    QString output_2d_dir=outfiles.at(0);
//    if(!output_2d_dir.endsWith("/")){
//        output_2d_dir = output_2d_dir+"/";
//    }
//    QStringList list=input_swc.split("/");
//    QString flag=list.last(); QStringList list1=flag.split(".");// you don't need to add 1 to find the string you want in input_dir
//    QString flag1=list1.first();//dont need to match list index
//    //printf("______________:%s\n",output_2d_dir.data());
//    qDebug()<<input_swc;
//    qDebug("number:%s",qPrintable(flag1));
//    NeuronTree nt_crop_sorted=readSWC_file(input_swc);
//    Image4DSimple * p4dImage = callback.loadImage((char *)(qPrintable(input_image) ));
//    int nChannel = p4dImage->getCDim();

//    V3DLONG mysz[4];
//    mysz[0] = p4dImage->getXDim();
//    mysz[1] = p4dImage->getYDim();
//    mysz[2] = p4dImage->getZDim();
//    mysz[3] = nChannel;
//    cout<<mysz[0]<<endl<<mysz[1]<<endl<<mysz[2]<<endl<<mysz[3]<<endl;
//    unsigned char *data1d_crop=p4dImage->getRawDataAtChannel(nChannel);
//    //printf("+++++++++++:%p\n",p4dImage);

//   V3DLONG pagesz = mysz[0]*mysz[1]*mysz[2];
//   unsigned char* data1d_mask = 0;
//   data1d_mask = new unsigned char [pagesz];
//   memset(data1d_mask,0,pagesz*sizeof(unsigned char));
//   double margin=0;//by PHC 20170531
//   ComputemaskImage(nt_crop_sorted, data1d_mask, mysz[0], mysz[1], mysz[2], margin);
//   //QString labelSaveString = pathname + ".v3draw_label.tif";
//   //simple_saveimage_wrapper(callback, labelSaveString.toLatin1().data(),(unsigned char *)data1d_mask, mysz, 1);

//   V3DLONG stacksz =mysz[0]*mysz[1];
//   unsigned char *image_mip=0;
//   image_mip = new unsigned char [stacksz];//2D orignal image
//   unsigned char *label_mip=0;
//   label_mip = new unsigned char [stacksz];//2D annotation
//   for(V3DLONG iy = 0; iy < mysz[1]; iy++)
//   {
//       V3DLONG offsetj = iy*mysz[0];
//       for(V3DLONG ix = 0; ix < mysz[0]; ix++)
//       {
//           int max_mip = 0;
//           int max_label = 0;
//           for(V3DLONG iz = 0; iz < mysz[2]; iz++)
//           {
//               V3DLONG offsetk = iz*mysz[1]*mysz[0];
//               if(data1d_crop[offsetk + offsetj + ix] >= max_mip)
//               {
//                   image_mip[iy*mysz[0] + ix] = data1d_crop[offsetk + offsetj + ix];
//                   max_mip = data1d_crop[offsetk + offsetj + ix];
//               }
//               if(data1d_mask[offsetk + offsetj + ix] >= max_label)
//               {
//                   label_mip[iy*mysz[0] + ix] = data1d_mask[offsetk + offsetj + ix];
//                   max_label = data1d_mask[offsetk + offsetj + ix];
//               }
//           }
//       }
//   }
//   unsigned char* data1d_2D = 0;
//   data1d_2D = new unsigned char [3*stacksz];//3 channels image
//   for(V3DLONG i=0; i<stacksz; i++)
//       data1d_2D[i] = image_mip[i];

//   for(V3DLONG i=0; i<stacksz; i++)
//   {
//       data1d_2D[i+stacksz] = (label_mip[i] ==255) ? 255: image_mip[i];
//   }
//   for(V3DLONG i=0; i<stacksz; i++)
//       data1d_2D[i+2*stacksz] = image_mip[i];

//   mysz[2] = 1;
//   mysz[3] = 3;
//   QString mipoutpuut = output_2d_dir +flag1+"_"+"mip.tif";
//   simple_saveimage_wrapper(callback,mipoutpuut.toStdString().c_str(),(unsigned char *)data1d_2D,mysz,1);
//   if(data1d_crop) {delete [] data1d_crop; data1d_crop=0;}
//   if(data1d_mask) {delete [] data1d_mask; data1d_mask=0;}
//   if(data1d_2D) {delete [] data1d_2D; data1d_2D=0;}
//   if(image_mip) {delete [] image_mip; image_mip=0;}
//   if(label_mip) {delete [] label_mip; label_mip=0;}
//   //listNeuron.clear();
//}

void missing_branch_detection(const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback, QWidget *parent)
{
    vector<char*> infiles, inparas, outfiles;
    if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
    if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
    if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

    QString image_file=infiles.at(1);
//    QString swc_file = infiles.at(1);
    QString output_dir=outfiles.at(0);

    QString swc_file = infiles.at(0);

    QStringList list=swc_file.split("/");
    QString flag=list.last(); QStringList list1=flag.split(".");// you don't need to add 1 to find the string you want in input_dir
    QString flag1=list1.first();
    QString output_missing = output_dir+"/"+flag1+"_missing.apo";

    printf("welcome to use get_missing_branch\n");
    NeuronTree nt = readSWC_file(swc_file);

//    Image4DSimple * p4dImage = callback.loadImage((char *)(qPrintable(image_file) ));
//    int nChannel = p4dImage->getCDim();

//    V3DLONG sz[4];
//    sz[0] = p4dImage->getXDim();
//    sz[1] = p4dImage->getYDim();
//    sz[2] = p4dImage->getZDim();
//    sz[3] = nChannel;
    //unsigned char *data1d_crop=p4dImage->getRawDataAtChannel(nChannel);

    V3DLONG *in_zz;
    cout<<"image file name is "<<image_file.toStdString()<<endl;
    if(!callback.getDimTeraFly(image_file.toStdString(), in_zz))
    {
//        v3d_msg("Cannot load terafly images.",0);
        cout<<"can not load terafly images"<<endl;
    }
    cout<<"check1"<<endl;
    XYZ block_size=XYZ(100,100,20);
    block zcenter_block; // This is a block centered at (0,0,0)
    zcenter_block.small = 0-block_size/2;
    zcenter_block.large = block_size/2;

    LandmarkList candidates;

//    for(int i=0;i<nt.listNeuron.size();i++){
    for(int i=0;i<1;i++){
        float nx=nt.listNeuron.at(i).x;
        float ny=nt.listNeuron.at(i).y;
        float nz=nt.listNeuron.at(i).z;

        LandmarkList candi;
        block crop_block = offset_block(zcenter_block, XYZ(nx, ny, nz));

                    XYZ small=XYZ(crop_block.small);
                    XYZ large=XYZ(crop_block.large);
                    small.x = floor(small.x);
                    small.y = floor(small.y);
                    small.z = floor(small.z);
                    large.x = ceil(large.x)+1;
                    large.y = ceil(large.y)+1;
                    large.z = ceil(large.z)+1;
                    unsigned char * cropped_image = 0;
                    //cout<<"dim"<<small.x<<small.y<<small.z<<large.x<<large.y<<large.z<<endl;
                    cropped_image = callback.getSubVolumeTeraFly(image_file.toStdString(),
                                                                 small.x, large.x,
                                                                 small.y, large.y,
                                                                 small.z, large.z);
                    V3DLONG mysz[4];
                    mysz[0] = large.x-small.x;
                    mysz[1] = large.y-small.y;
                    mysz[2] = large.z-small.z;
                    mysz[3] = in_zz[3];

                    unsigned char * data1d_crop=cropped_image;
                    QString output_swc = output_dir+flag1+".eswc";
                    //QString output_swc1=output_dir+"/";
                    crop_swc_cuboid(nt, output_swc, crop_block);
                    NeuronTree nt_crop_sorted;
                    nt_crop_sorted=readSWC_file(output_swc);
                    int center=find_tip(nt_crop_sorted,mysz[0],mysz[1],mysz[2]);
                    float cx = nt_crop_sorted.listNeuron.at(center).x;
                    float cy = nt_crop_sorted.listNeuron.at(center).y;
                    float cz = nt_crop_sorted.listNeuron.at(center).z;

                    candi = get_missing_branches_menu(cropped_image,nt_crop_sorted,mysz[0],mysz[1],mysz[2]);

                    ImagePixelType pixtype = V3D_UINT16;
                    //candidates.append(candi);
//                    if(load_data(data1d_crop,candidates,pixtype,mysz,curwin))
//                    {
//                        QList <ImageMarker> final_pts;
//                        vector <long> final_ptsx,final_ptsy,final_ptsz;
//        //                vector <struct XYZ> final_pts_xyz;
//                        mean_shift_fun ms;
//                        qDebug() << "mean shift fun instantiated";
//                        for(V3DLONG i=0; i<candidates.size(); i++)
//                        {
//                            vector <float> final_pt;
//                            ImageMarker final_pt_mk;
//                            qDebug() << "mean shift not called";
//                            final_pt = ms.mean_shift_center(i,20);
//                            qDebug() << "mean shift called";
//                            final_pt_mk.x = final_pt[0];
//                            final_pt_mk.y = final_pt[1];
//                            final_pt_mk.z = final_pt[2];
//                            if(find(final_ptsx.begin(),final_ptsx.end(),final_pt_mk.x)!=final_ptsx.end() ||
//                               find(final_ptsy.begin(),final_ptsy.end(),final_pt_mk.y)!=final_ptsy.end() ||
//                               find(final_ptsz.begin(),final_ptsz.end(),final_pt_mk.z)!=final_ptsz.end())
//                            {
//                                final_ptsx.push_back(long(final_pt[0]));
//                                final_ptsy.push_back(long(final_pt[1]));
//                                final_ptsz.push_back(long(final_pt[2]));
//                                final_pts.push_back(final_pt_mk);
//                            }
//        //                    struct XYZ final_pt_xyz = XYZ(final_pt[0],final_pt[1],final_pt[2]);
//        //                    if(find(final_pts_xyz.begin(),final_pts_xyz.end(),final_pt_xyz) != final_pts_xyz.end()) final_pts_xyz.push_back(final_pt_xyz);
//                        }
//                        qDebug() << "Final points size:" << final_pts.size();

//                    }
    }



//    unsigned int Vsize=50;
//    QList<CellAPO> missing;

//    for(int i = 0; i <candidates.size();i++){
//        CellAPO m;
//        m.x = candidates.at(i).x;
//        m.y = candidates.at(i).y;
//        m.z = candidates.at(i).z;
//        m.color.r=255;
//        m.color.g=255;
//        m.color.b=255;
//        m.volsize = Vsize;
//        missing.push_back(m);
//    }

//    writeAPO_file(output_missing,missing);
}




LandmarkList get_missing_branches_menu(unsigned char *data1d_crop, NeuronTree nt, long mysz0,long mysz1,long mysz2)
{
    LandmarkList candidates_m;

    // Get Neuron
    QList<NeuronSWC> neuron = nt.listNeuron;

    // Get Image Data
//    int nChannel = p4DImage->getCDim();

//    V3DLONG mysz[4];
//    mysz[0] = p4DImage->getXDim();
//    mysz[1] = p4DImage->getYDim();
//    mysz[2] = p4DImage->getZDim();
//    mysz[3] = nChannel;
//    cout<<mysz[0]<<endl<<mysz[1]<<endl<<mysz[2]<<endl<<mysz[3]<<endl;
//    unsigned char *data1d_crop=p4DImage->getRawDataAtChannel(nChannel);
//    printf("+++++++++++:%p\n",p4DImage);

    vector<long> ids;
    vector<long> parents;
    // Reorder tree ids so that neuron.at(i).n=i+1
    for(V3DLONG i=0;i<neuron.size();i++)
    {
        ids.push_back(neuron.at(i).n);
    }
    for(V3DLONG i=0;i<neuron.size();i++)
    {
        neuron[i].n=i+1;
        if(neuron.at(i).pn !=-1)
        {
            neuron[i].pn=find(ids.begin(), ids.end(),neuron.at(i).pn) - ids.begin()+1;
            parents.push_back(neuron.at(i).pn);
        }
    }

    // Get foreground points in a shell around SWC
    V3DLONG radius = 40;
    vector <struct XYZ> candidates;
    vector <struct XYZ> locswc;
    vector <struct XYZ> final_pts;
    for(V3DLONG id=0; id<neuron.size(); id++)
    {
        locswc.push_back(XYZ(neuron.at(id).x,neuron.at(id).y,neuron.at(id).z));
    }

    for(V3DLONG id=0; id<neuron.size(); id++)
    {
        V3DLONG nodex = neuron.at(id).x;
        V3DLONG nodey = neuron.at(id).y;
        V3DLONG nodez = neuron.at(id).z;
        //qDebug() << nodex << nodey << nodez;
        for(double theta=0; theta<PI; theta+=0.8) // Check the step in function of the radius
        {
            //qDebug() << theta;
            for(double phi=0; phi<2*PI; phi+=0.8)
            {
                //qDebug() << phi;
                struct XYZ shellp = XYZ(nodex+round(radius*sin(theta)*cos(phi)),nodey+round(radius*sin(theta)*sin(phi)),nodez+round(radius*cos(theta)/5));
                if((shellp.x<0) || (shellp.x>=mysz0) || (shellp.y<0) || (shellp.y>=mysz1) || (shellp.z<0) || (shellp.z>=mysz2)){continue;} // out of image border
                for(int i=0; i<neuron.size(); i++)
//                for(int i=1; i<80; i++)
                {
                    //qDebug() << i;
                    // Checks whether a point is in a shell of radius 20<r<40 pixels and if intensity >40
                    if(dist_L2(shellp,locswc.at(i))>=20 && data1d_crop[V3DLONG(shellp.z*mysz0*mysz1+shellp.y*mysz0+shellp.x)]>=40) {
                        candidates.push_back(shellp);
                    }
                }
            }
        }
     }
    for (V3DLONG i=0; i<candidates.size(); i++)
    {
        LocationSimple candidate;
        candidate.x = candidates.at(i).x;
        candidate.y = candidates.at(i).y;
        candidate.z = candidates.at(i).z;

        candidates_m.push_back(candidate);
    }


    qDebug() << "Candidates vector size is:" << candidates.size();
    return candidates_m;




}
