// fastmarching_dt.h       2012-02-23 by Hang Xiao

#ifndef __FASTMARCHING_DT_H__
#define __FASTMARCHING_DT_H__

#include <iostream>
#include <sstream>
#include <map>
#include "v3d_interface.h"
#include "heap.h"
#include "my_surf_objs.h"
#include "fastmarching_macro.h"
#include "volimg_proc.h"

using namespace std;

#ifndef ABS
#define ABS(x) ((x) > 0 ? (x) : (-(x)))
#endif

/******************************************************************************
 * Fast marching based distance transformation
 * 
 * Input : inimg1d     original input grayscale image
 *         cnn_type    the connection type
 *         bkg_thresh  the background threshold, less or equal then bkg_thresh will considered as background
 *
 * Output : phi       distance tranformed output image
 *
 * Notice : 
 * 1. the input pixel number should not be larger than 2G if sizeof(long) == 4
 * 2. The background point is of intensity 0
 * *****************************************************************************/

template<class T>bool adaptiveLocalTheshold(T * inimg1d, unsigned char * & mask, V3DLONG *sz)
{
    long sz0 = sz[0];
    long sz01 = sz[0] * sz[1];
    long tol_sz= sz[0]* sz[1]*  sz[2];
    //long max_r= sz[0]/2;
    if(mask ==0) mask= new unsigned char [tol_sz];
    double *normal1d= new double [tol_sz];

    double imgAve, imgStd;
    mean_and_std(inimg1d,tol_sz,imgAve,imgStd);
    double lamda=0.5;
    for(long ind=0; ind< tol_sz; ++ind)
    {
        V3DLONG zz=ind/sz01;
        V3DLONG yy=(ind-zz*sz01)/sz0;
        V3DLONG xx=ind-zz*sz01-yy*sz0;
        V3DLONG rr_max=sz[0]/2;
        qDebug()<<"x y z :"<< xx<< yy<<zz;
        double res_mean=0, res_std=0, res_mass=0, res_size=0;
        for(V3DLONG rr=10;rr<rr_max;rr+=5)
        {
            res_mean=0, res_std=0, res_mass=0, res_size=0;
            qDebug()<<"rr="<<rr;
            //v3d_msg("check rr", 1);
            //compute the mean and std of current window
            V3DLONG xs,xe,ys,ye,zs,ze;
            xs = xx-rr; if (xs<0) xs=0;
            xe = xx+rr; if (xe>sz[0]) xe = sz[0]-1;
            ys = yy-rr; if (ys<0) ys=0;
            ye = yy+rr; if (ye>sz[1]) ye = sz[1]-1;
            zs = zz-rr; if (zs<0) zs=0;
            ze = zz+rr; if (ze>sz[2]) ze = sz[2]-1;

            double r2=double(rr+1)*(rr+1);
            for (V3DLONG k=zs;k<=ze;k++)
            {
                double cur_dk = (k-zz)*(k-zz);
                for (V3DLONG j=ys;j<=ye;j++)
                {
                    double cur_dj = (j-yy)*(j-yy);
                    double cur_d = cur_dk + cur_dj;
                    if (cur_d>r2) continue;
                    for (V3DLONG i=xs;i<=xe;i++)
                    {
                        double cur_di = (i-xx)*(i-xx);
                        cur_d = cur_dk + cur_dj + cur_di;
                        if (cur_d>r2) continue;

                        double cur_v = double(inimg1d[k*sz01+j*sz0+i]);
                        //qDebug()<<"cur_v="<<cur_v;
                        //if (res_peak<cur_v) res_peak = cur_v;
                        res_size++;
                        res_mass += cur_v;
                        res_std += cur_v*cur_v; //use the incremental formula
                        //qDebug()<<"inter res_std:"<<res_std;
                        //v3d_msg("check std", 1);

                    }
                }
            }
            res_mean = res_mass/res_size;
            res_std = sqrt(res_std/res_size - res_mean*res_mean);
            qDebug()<<"res_std="<<res_std;
            qDebug()<<"threshold std="<<lamda*imgStd;

            if(res_std>= lamda*imgStd) break;
        }
        if(res_std<lamda*imgStd) res_std=lamda*imgStd;
        // save the normalized value to 1d array
        normal1d[ind]=(inimg1d[ind]-res_mean)/res_std;
        }

    double low_thresh=0.5, high_thresh=1;
    for(long ind=0; ind< tol_sz; ++ind)
    {
        mask[ind]=0;
        V3DLONG zz=ind/sz01;
        V3DLONG yy=(ind-zz*sz01)/sz0;
        V3DLONG xx=ind-zz*sz01-yy*sz0;

        V3DLONG xs,xe,ys,ye,zs,ze;
        xs = xx-1; if (xs<0) xs=0;
        xe = xx+1; if (xe>sz[0]) xe = sz[0]-1;
        ys = yy-1; if (ys<0) ys=0;
        ye = yy+1; if (ye>sz[1]) ye = sz[1]-1;
        zs = zz-1; if (zs<0) zs=0;
        ze = zz+1; if (ze>sz[2]) ze = sz[2]-1;
        if(normal1d[ind]>low_thresh)
        {
            for (V3DLONG k=zs;k<=ze;k++)
            {
                double cur_dk = (k-zz)*(k-zz);
                for (V3DLONG j=ys;j<=ye;j++)
                {
                    double cur_dj = (j-yy)*(j-yy);
                    double cur_d = cur_dk + cur_dj;
                    if (cur_d>1) continue;
                    for (V3DLONG i=xs;i<=xe;i++)
                    {
                        double cur_di = (i-xx)*(i-xx);
                        cur_d = cur_dk + cur_dj + cur_di;
                        if (cur_d>1) continue;
                        if(normal1d[k*sz01+j*sz0+i]>high_thresh) mask[ind]=255;

                    }

                }
            }
        }
    }

    if (normal1d) {delete [] normal1d; normal1d=0; }
    return true;


}

template<class T> bool fastmarching_dt(T * inimg1d, float * &phi, int sz0, int sz1, int sz2, int cnn_type = 3, int bkg_thresh = 0)
{
	enum{ALIVE = -1, TRIAL = 0, FAR = 1};

	long tol_sz = sz0 * sz1 * sz2;
	long sz01 = sz0 * sz1;
	//int cnn_type = 3;  // ?

	if(phi == 0) phi = new float[tol_sz]; 

	char * state = new char[tol_sz];
    //unsigned char *mask= new unsigned char [tol_sz];
	int bkg_count = 0;                          // for process counting
	int bdr_count = 0;                          // for process counting
	for(long i = 0; i < tol_sz; i++)
	{
		if(inimg1d[i] <= bkg_thresh)
		{
			phi[i] = inimg1d[i];
            //mask[i]=255;
			state[i] = ALIVE;
			//cout<<"+";cout.flush();
			bkg_count++;
		}
		else
		{
            //mask[i]=0;
			phi[i] = INF;
			state[i] = FAR;
		}
	}
	cout<<endl;

	BasicHeap<HeapElem> heap;
	map<long, HeapElem*> elems;

	// init heap
	{
		long i = -1, j = -1, k = -1;
		for(long ind = 0; ind < tol_sz; ind++)
		{
			i++; if(i%sz0 == 0){i=0; j++; if(j%sz1==0){j=0; k++;}}
			if(state[ind] == ALIVE)
			{
				for(int kk = -1; kk <= 1; kk++)
				{
					long k2 = k+kk;
					if(k2 < 0 || k2 >= sz2) continue;
					for(int jj = -1; jj <= 1; jj++)
					{
						long j2 = j+jj;
						if(j2 < 0 || j2 >= sz1) continue;
						for(int ii = -1; ii <=1; ii++)
						{
							long i2 = i+ii;
							if(i2 < 0 || i2 >= sz0) continue;
							int offset = ABS(ii) + ABS(jj) + ABS(kk);
							if(offset == 0 || offset > cnn_type) continue;
							long ind2 = k2 * sz01 + j2 * sz0 + i2;
							if(state[ind2] == FAR)
							{
								long min_ind = ind;
								// get minimum Alive point around ind2
								if(phi[min_ind] > 0.0)
								{
									for(int kkk = -1; kkk <= 1; kkk++)
									{
										long k3 = k2 + kkk;
										if(k3 < 0 || k3 >= sz2) continue;
										for(int jjj = -1; jjj <= 1; jjj++)
										{
											long j3 = j2 + jjj;
											if(j3 < 0 || j3 >= sz1) continue;
											for(int iii = -1; iii <= 1; iii++)
											{
												long i3 = i2 + iii;
												if(i3 < 0 || i3 >= sz0) continue;
												int offset2 = ABS(iii) + ABS(jjj) + ABS(kkk);
												if(offset2 == 0 || offset2 > cnn_type) continue;
												long ind3 = k3 * sz01 + j3 * sz0 + i3;
												if(state[ind3] == ALIVE && phi[ind3] < phi[min_ind]) min_ind = ind3;
											}
										}
									}
								}
								// over
								phi[ind2] = phi[min_ind] + inimg1d[ind2];
								state[ind2] = TRIAL;
								HeapElem * elem = new HeapElem(ind2, phi[ind2]);
								heap.insert(elem);
								elems[ind2] = elem;
								bdr_count++;
							}
						}
					}
				}
			}
		}
	}

	cout<<"bkg_count = "<<bkg_count<<" ("<<bkg_count/(double)tol_sz<<")"<<endl;
	cout<<"bdr_count = "<<bdr_count<<" ("<<bdr_count/(double)tol_sz<<")"<<endl;
	cout<<"elems.size() = "<<elems.size()<<endl;
	// loop
	int time_counter = bkg_count;
	double process1 = 0;
	while(!heap.empty())
	{
		double process2 = (time_counter++)*100000.0/tol_sz;
		if(process2 - process1 >= 100) {cout<<"\r"<<((int)process2)/1000.0<<"%";cout.flush(); process1 = process2;
		}

		HeapElem* min_elem = heap.delete_min();
		elems.erase(min_elem->img_ind);

		long min_ind = min_elem->img_ind;
		delete min_elem;

		state[min_ind] = ALIVE;
		int i = min_ind % sz0; 
		int j = (min_ind/sz0) % sz1; 
		int k = (min_ind/sz01) % sz2;

		int w, h, d;
		for(int kk = -1; kk <= 1; kk++)
		{
			d = k+kk;
			if(d < 0 || d >= sz2) continue;
			for(int jj = -1; jj <= 1; jj++)
			{
				h = j+jj;
				if(h < 0 || h >= sz1) continue;
				for(int ii = -1; ii <= 1; ii++)
				{
					w = i+ii;
					if(w < 0 || w >= sz0) continue;
					int offset = ABS(ii) + ABS(jj) + ABS(kk);
					if(offset == 0 || offset > cnn_type) continue;
					long index = d*sz01 + h*sz0 + w;

					if(state[index] != ALIVE)
					{
						float new_dist = phi[min_ind] + inimg1d[index] * sqrt(double(offset));

						if(state[index] == FAR)
						{
							phi[index] = new_dist;
							HeapElem * elem = new HeapElem(index, phi[index]);
							heap.insert(elem);
							elems[index] = elem;
							state[index] = TRIAL;
						}
						else if(state[index] == TRIAL)
						{
							if(phi[index] > new_dist)
							{
								phi[index] = new_dist;
								HeapElem * elem = elems[index];
								heap.adjust(elem->heap_id, phi[index]);
							}
						}
					}
				}
			}
		}
	}
	//END_CLOCK;
	assert(elems.empty());
	if(state) {delete [] state; state = 0;}
	return true;
}

template<class T> bool fastmarching_dt(T * inimg1d, float * &phi, unsigned char * &mask,int sz0, int sz1, int sz2, int cnn_type = 3, int bkg_thresh = 0)
{
    enum{ALIVE = -1, TRIAL = 0, FAR = 1};

    long tol_sz = sz0 * sz1 * sz2;
    long sz01 = sz0 * sz1;
    //int cnn_type = 3;  // ?

    if(phi == 0) phi = new float[tol_sz];

    char * state = new char[tol_sz];
    if(mask==0) mask= new unsigned char [tol_sz];
    int bkg_count = 0;                          // for process counting
    int bdr_count = 0;                          // for process counting
    for(long i = 0; i < tol_sz; i++)
    {
        if(inimg1d[i] <= bkg_thresh)
        {
            phi[i] = inimg1d[i];
            mask[i]=255;
            state[i] = ALIVE;
            //cout<<"+";cout.flush();
            bkg_count++;
        }
        else
        {
            mask[i]=0;
            phi[i] = INF;
            state[i] = FAR;
        }
    }
    cout<<endl;

    BasicHeap<HeapElem> heap;
    map<long, HeapElem*> elems;

    // init heap
    {
        long i = -1, j = -1, k = -1;
        for(long ind = 0; ind < tol_sz; ind++)
        {
            i++; if(i%sz0 == 0){i=0; j++; if(j%sz1==0){j=0; k++;}}
            if(state[ind] == ALIVE)
            {
                for(int kk = -1; kk <= 1; kk++)
                {
                    long k2 = k+kk;
                    if(k2 < 0 || k2 >= sz2) continue;
                    for(int jj = -1; jj <= 1; jj++)
                    {
                        long j2 = j+jj;
                        if(j2 < 0 || j2 >= sz1) continue;
                        for(int ii = -1; ii <=1; ii++)
                        {
                            long i2 = i+ii;
                            if(i2 < 0 || i2 >= sz0) continue;
                            int offset = ABS(ii) + ABS(jj) + ABS(kk);
                            if(offset == 0 || offset > cnn_type) continue;
                            long ind2 = k2 * sz01 + j2 * sz0 + i2;
                            if(state[ind2] == FAR)
                            {
                                long min_ind = ind;
                                // get minimum Alive point around ind2
                                if(phi[min_ind] > 0.0)
                                {
                                    for(int kkk = -1; kkk <= 1; kkk++)
                                    {
                                        long k3 = k2 + kkk;
                                        if(k3 < 0 || k3 >= sz2) continue;
                                        for(int jjj = -1; jjj <= 1; jjj++)
                                        {
                                            long j3 = j2 + jjj;
                                            if(j3 < 0 || j3 >= sz1) continue;
                                            for(int iii = -1; iii <= 1; iii++)
                                            {
                                                long i3 = i2 + iii;
                                                if(i3 < 0 || i3 >= sz0) continue;
                                                int offset2 = ABS(iii) + ABS(jjj) + ABS(kkk);
                                                if(offset2 == 0 || offset2 > cnn_type) continue;
                                                long ind3 = k3 * sz01 + j3 * sz0 + i3;
                                                if(state[ind3] == ALIVE && phi[ind3] < phi[min_ind]) min_ind = ind3;
                                            }
                                        }
                                    }
                                }
                                // over
                                phi[ind2] = phi[min_ind] + inimg1d[ind2];
                                state[ind2] = TRIAL;
                                HeapElem * elem = new HeapElem(ind2, phi[ind2]);
                                heap.insert(elem);
                                elems[ind2] = elem;
                                bdr_count++;
                            }
                        }
                    }
                }
            }
        }
    }

    cout<<"bkg_count = "<<bkg_count<<" ("<<bkg_count/(double)tol_sz<<")"<<endl;
    cout<<"bdr_count = "<<bdr_count<<" ("<<bdr_count/(double)tol_sz<<")"<<endl;
    cout<<"elems.size() = "<<elems.size()<<endl;
    // loop
    int time_counter = bkg_count;
    double process1 = 0;
    while(!heap.empty())
    {
        double process2 = (time_counter++)*100000.0/tol_sz;
        if(process2 - process1 >= 100) {cout<<"\r"<<((int)process2)/1000.0<<"%";cout.flush(); process1 = process2;
        }

        HeapElem* min_elem = heap.delete_min();
        elems.erase(min_elem->img_ind);

        long min_ind = min_elem->img_ind;
        delete min_elem;

        state[min_ind] = ALIVE;
        int i = min_ind % sz0;
        int j = (min_ind/sz0) % sz1;
        int k = (min_ind/sz01) % sz2;

        int w, h, d;
        for(int kk = -1; kk <= 1; kk++)
        {
            d = k+kk;
            if(d < 0 || d >= sz2) continue;
            for(int jj = -1; jj <= 1; jj++)
            {
                h = j+jj;
                if(h < 0 || h >= sz1) continue;
                for(int ii = -1; ii <= 1; ii++)
                {
                    w = i+ii;
                    if(w < 0 || w >= sz0) continue;
                    int offset = ABS(ii) + ABS(jj) + ABS(kk);
                    if(offset == 0 || offset > cnn_type) continue;
                    long index = d*sz01 + h*sz0 + w;

                    if(state[index] != ALIVE)
                    {
                        float new_dist = phi[min_ind] + inimg1d[index] * sqrt(double(offset));

                        if(state[index] == FAR)
                        {
                            phi[index] = new_dist;
                            HeapElem * elem = new HeapElem(index, phi[index]);
                            heap.insert(elem);
                            elems[index] = elem;
                            state[index] = TRIAL;
                        }
                        else if(state[index] == TRIAL)
                        {
                            if(phi[index] > new_dist)
                            {
                                phi[index] = new_dist;
                                HeapElem * elem = elems[index];
                                heap.adjust(elem->heap_id, phi[index]);
                            }
                        }
                    }
                }
            }
        }
    }
    //END_CLOCK;
    assert(elems.empty());
    if(state) {delete [] state; state = 0;}
    return true;
}

template<class T> bool fastmarching_dt_XY(T * inimg1d, float * &phi, int sz0, int sz1, int sz2, int cnn_type = 3, int bkg_thresh = 0)
{
	enum{ALIVE = -1, TRIAL = 0, FAR = 1};
	
	long tol_sz = sz0 * sz1 * sz2;
	long sz01 = sz0 * sz1;
	//int cnn_type = 3;  // ?
	
	if(phi == 0) phi = new float[tol_sz]; 
	char * state = new char[tol_sz];
	int bkg_count = 0;                          // for process counting
	int bdr_count = 0;                          // for process counting
	for(long i = 0; i < tol_sz; i++)
	{
		if(inimg1d[i] <= bkg_thresh)
		{
			phi[i] = inimg1d[i];
			state[i] = ALIVE;
			//cout<<"+";cout.flush();
			bkg_count++;
		}
		else
		{
			phi[i] = INF;
			state[i] = FAR;
		}
	}
	cout<<endl;
	
	BasicHeap<HeapElem> heap;
	map<long, HeapElem*> elems;
	
	// init heap
	{
		long i = -1, j = -1, k = -1;
		for(long ind = 0; ind < tol_sz; ind++)
		{
			i++; if(i%sz0 == 0){i=0; j++; if(j%sz1==0){j=0; k++;}}
			if(state[ind] == ALIVE)
			{
				for(int kk = 0; kk <= 0; kk++)
				{
					long k2 = k+kk;
					if(k2 < 0 || k2 >= sz2) continue;
					for(int jj = -1; jj <= 1; jj++)
					{
						long j2 = j+jj;
						if(j2 < 0 || j2 >= sz1) continue;
						for(int ii = -1; ii <=1; ii++)
						{
							long i2 = i+ii;
							if(i2 < 0 || i2 >= sz0) continue;
							int offset = ABS(ii) + ABS(jj) + ABS(kk);
							if(offset == 0 || offset > cnn_type) continue;
							long ind2 = k2 * sz01 + j2 * sz0 + i2;
							if(state[ind2] == FAR)
							{
								long min_ind = ind;
								// get minimum Alive point around ind2
								if(phi[min_ind] > 0.0)
								{
									for(int kkk = 0; kkk <= 0; kkk++)
									{
										long k3 = k2 + kkk;
										if(k3 < 0 || k3 >= sz2) continue;
										for(int jjj = -1; jjj <= 1; jjj++)
										{
											long j3 = j2 + jjj;
											if(j3 < 0 || j3 >= sz1) continue;
											for(int iii = -1; iii <= 1; iii++)
											{
												long i3 = i2 + iii;
												if(i3 < 0 || i3 >= sz0) continue;
												int offset2 = ABS(iii) + ABS(jjj) + ABS(kkk);
												if(offset2 == 0 || offset2 > cnn_type) continue;
												long ind3 = k3 * sz01 + j3 * sz0 + i3;
												if(state[ind3] == ALIVE && phi[ind3] < phi[min_ind]) min_ind = ind3;
											}
										}
									}
								}
								// over
								phi[ind2] = phi[min_ind] + inimg1d[ind2];
								state[ind2] = TRIAL;
								HeapElem * elem = new HeapElem(ind2, phi[ind2]);
								heap.insert(elem);
								elems[ind2] = elem;
								bdr_count++;
							}
						}
					}
				}
			}
		}
	}
	
	cout<<"bkg_count = "<<bkg_count<<" ("<<bkg_count/(double)tol_sz<<")"<<endl;
	cout<<"bdr_count = "<<bdr_count<<" ("<<bdr_count/(double)tol_sz<<")"<<endl;
	cout<<"elems.size() = "<<elems.size()<<endl;
	// loop
	int time_counter = bkg_count;
	double process1 = 0;
	while(!heap.empty())
	{
		double process2 = (time_counter++)*100000.0/tol_sz;
		if(process2 - process1 >= 10) {cout<<"\r"<<((int)process2)/1000.0<<"%";cout.flush(); process1 = process2;
			//SAVE_PHI_IMAGE(phi, sz0, sz1, sz2, string("phi") + num2str((int)process1) + ".tif");
		}
		
		HeapElem* min_elem = heap.delete_min();
		elems.erase(min_elem->img_ind);
		
		long min_ind = min_elem->img_ind;
		delete min_elem;
		
		state[min_ind] = ALIVE;
		int i = min_ind % sz0; 
		int j = (min_ind/sz0) % sz1; 
		int k = (min_ind/sz01) % sz2;
		
		int w, h, d;
		for(int kk = 0; kk <= 0; kk++)
		{
			d = k+kk;
			if(d < 0 || d >= sz2) continue;
			for(int jj = -1; jj <= 1; jj++)
			{
				h = j+jj;
				if(h < 0 || h >= sz1) continue;
				for(int ii = -1; ii <= 1; ii++)
				{
					w = i+ii;
					if(w < 0 || w >= sz0) continue;
					int offset = ABS(ii) + ABS(jj) + ABS(kk);
					if(offset == 0 || offset > cnn_type) continue;
					long index = d*sz01 + h*sz0 + w;
					
					if(state[index] != ALIVE)
					{
						float new_dist = phi[min_ind] + inimg1d[index] * sqrt(double(offset));
						
						if(state[index] == FAR)
						{
							phi[index] = new_dist;
							HeapElem * elem = new HeapElem(index, phi[index]);
							heap.insert(elem);
							elems[index] = elem;
							state[index] = TRIAL;
						}
						else if(state[index] == TRIAL)
						{
							if(phi[index] > new_dist)
							{
								phi[index] = new_dist;
								HeapElem * elem = elems[index];
								heap.adjust(elem->heap_id, phi[index]);
							}
						}
					}
				}
			}
		}
	}
	
	assert(elems.empty());
	if(state) {delete [] state; state = 0;}
	return true;
}

/******************************************************************************
 * Fast marching based distance transformation tree
 * 
 * Input : inimg1d     original input grayscale image
 *         cnn_type    the connection type
 *         bkg_thresh  the background threshold, less or equal then bkg_thresh will considered as background
 *
 * Output : phi       distance tranformed output image
 *
 * Notice : 
 * 1. the input pixel number should not be larger than 2G if sizeof(long) == 4
 * 2. The background point is of intensity 0
 * *****************************************************************************/

template<class T> bool fastmarching_dt_tree(T * inimg1d, vector<MyMarker*> &outtree, int sz0, int sz1, int sz2, int cnn_type = 3, int bkg_thresh = 0)
{
	cout << " ... cell body detecting: bkg thres: " << bkg_thresh << endl;

	enum{ALIVE = -1, TRIAL = 0, FAR = 1};

	long tol_sz = sz0 * sz1 * sz2;
	long sz01 = sz0 * sz1;
	//int cnn_type = 3;  // ?

	float * phi = new float[tol_sz]; 
	long * parent = new long[tol_sz]; for(long ind = 0; ind < tol_sz; ind++) parent[ind] = ind;
	char * state = new char[tol_sz];
	int bkg_count = 0;                          // for process counting
	int bdr_count = 0;                          // for process counting
	for(long i = 0; i < tol_sz; i++)
	{
		if(inimg1d[i] <= bkg_thresh)
		{
			phi[i] = inimg1d[i];
			state[i] = ALIVE;
			//cout<<"+";cout.flush();
			bkg_count++;
		}
		else
		{
			phi[i] = INF;
			state[i] = FAR;
		}
	}
	cout<<endl;

	BasicHeap<HeapElem> heap;
	map<long, HeapElem*> elems;

	// init heap
	{
		long i = -1, j = -1, k = -1;
		for(long ind = 0; ind < tol_sz; ind++)
		{
			i++; if(i%sz0 == 0){i=0; j++; if(j%sz1==0){j=0; k++;}}
			if(state[ind] == ALIVE)
			{
				for(int kk = -1; kk <= 1; kk++)
				{
					long k2 = k+kk;
					if(k2 < 0 || k2 >= sz2) continue;
					for(int jj = -1; jj <= 1; jj++)
					{
						long j2 = j+jj;
						if(j2 < 0 || j2 >= sz1) continue;
						for(int ii = -1; ii <=1; ii++)
						{
							long i2 = i+ii;
							if(i2 < 0 || i2 >= sz0) continue;
							int offset = ABS(ii) + ABS(jj) + ABS(kk);
							if(offset == 0 || offset > cnn_type) continue;
							long ind2 = k2 * sz01 + j2 * sz0 + i2;
							if(state[ind2] == FAR)
							{
								long min_ind = ind;
								// get minimum Alive point around ind2
								if(phi[min_ind] > 0.0)
								{
									for(int kkk = -1; kkk <= 1; kkk++)
									{
										long k3 = k2 + kkk;
										if(k3 < 0 || k3 >= sz2) continue;
										for(int jjj = -1; jjj <= 1; jjj++)
										{
											long j3 = j2 + jjj;
											if(j3 < 0 || j3 >= sz1) continue;
											for(int iii = -1; iii <= 1; iii++)
											{
												long i3 = i2 + iii;
												if(i3 < 0 || i3 >= sz0) continue;
												int offset2 = ABS(iii) + ABS(jjj) + ABS(kkk);
												if(offset2 == 0 || offset2 > cnn_type) continue;
												long ind3 = k3 * sz01 + j3 * sz0 + i3;
												if(state[ind3] == ALIVE && phi[ind3] < phi[min_ind]) min_ind = ind3;
											}
										}
									}
								}
								// over
								phi[ind2] = phi[min_ind] + inimg1d[ind2];
								state[ind2] = TRIAL;
								parent[ind2] = min_ind;
								HeapElem * elem = new HeapElem(ind2, phi[ind2]);
								heap.insert(elem);
								elems[ind2] = elem;
								bdr_count++;
							}
						}
					}
				}
			}
		}
	}

	cout<<"bkg_count = "<<bkg_count<<" ("<<bkg_count/(double)tol_sz<<")"<<endl;
	cout<<"bdr_count = "<<bdr_count<<" ("<<bdr_count/(double)tol_sz<<")"<<endl;
	cout<<"elems.size() = "<<elems.size()<<endl;
	// loop
	int time_counter = bkg_count;
	double process1 = 0;
	while(!heap.empty())
	{
		double process2 = (time_counter++)*100000.0/tol_sz;
		if(process2 - process1 >= 10) {cout<<"\r"<<((int)process2)/1000.0<<"%";cout.flush(); process1 = process2;
			//SAVE_PHI_IMAGE(phi, sz0, sz1, sz2, string("phi") + num2str((int)process1) + ".tif");
		}

		HeapElem* min_elem = heap.delete_min();
		elems.erase(min_elem->img_ind);

		long min_ind = min_elem->img_ind;
		delete min_elem;

		state[min_ind] = ALIVE;
		int i = min_ind % sz0; 
		int j = (min_ind/sz0) % sz1; 
		int k = (min_ind/sz01) % sz2;

		int w, h, d;
		for(int kk = -1; kk <= 1; kk++)
		{
			d = k+kk;
			if(d < 0 || d >= sz2) continue;
			for(int jj = -1; jj <= 1; jj++)
			{
				h = j+jj;
				if(h < 0 || h >= sz1) continue;
				for(int ii = -1; ii <= 1; ii++)
				{
					w = i+ii;
					if(w < 0 || w >= sz0) continue;
					int offset = ABS(ii) + ABS(jj) + ABS(kk);
					if(offset == 0 || offset > cnn_type) continue;
					long index = d*sz01 + h*sz0 + w;

					if(state[index] != ALIVE)
					{
						float new_dist = phi[min_ind] + inimg1d[index] * sqrt(double(offset));

						if(state[index] == FAR)
						{
							phi[index] = new_dist;
							HeapElem * elem = new HeapElem(index, phi[index]);
							heap.insert(elem);
							elems[index] = elem;
							state[index] = TRIAL;
							parent[index] = min_ind;
						}
						else if(state[index] == TRIAL)
						{
							if(phi[index] > new_dist)
							{
								phi[index] = new_dist;
								HeapElem * elem = elems[index];
								heap.adjust(elem->heap_id, phi[index]);
								parent[index] = min_ind;
							}
						}
					}
				}
			}
		}
	}
	
	map<long, MyMarker*> marker_map;
	for(long ind = 0; ind < tol_sz; ind++)
	{
		if(inimg1d[ind] < bkg_thresh || parent[ind] == ind) continue;
		int i = ind % sz0;
		int j = ind / sz0 % sz1;
		int k = ind / sz01;
		MyMarker * marker = new MyMarker(i, j, k);
		outtree.push_back(marker);
		marker_map[ind] = marker;
	}
	for(int m = 0; m < outtree.size(); m++)
	{
		MyMarker * child_marker = outtree[m];
		MyMarker * parent_marker = marker_map[parent[child_marker->ind(sz0, sz01)]];
		child_marker->parent = parent_marker;
	}

	assert(elems.empty());
	if(phi){delete [] phi; phi = 0;}
	if(parent){delete [] parent; parent = 0;}
	if(state) {delete [] state; state = 0;}
	return true;
}

template<class T> bool fastmarching_dt_tree_XY(T * inimg1d, vector<MyMarker*> & outtree, int sz0, int sz1, int sz2, int cnn_type = 3, int bkg_thresh = 0)
{
	enum{ALIVE = -1, TRIAL = 0, FAR = 1};
	
	long tol_sz = sz0 * sz1 * sz2;
	long sz01 = sz0 * sz1;
	//int cnn_type = 3;  // ?
	
	float * phi = new float[tol_sz]; 
	long * parent = new long[tol_sz]; for(long ind = 0; ind < tol_sz; ind++) parent[ind] = ind;
	char * state = new char[tol_sz];
	int bkg_count = 0;                          // for process counting
	int bdr_count = 0;                          // for process counting
	for(long i = 0; i < tol_sz; i++)
	{
		if(inimg1d[i] <= bkg_thresh)
		{
			phi[i] = inimg1d[i];
			state[i] = ALIVE;
			//cout<<"+";cout.flush();
			bkg_count++;
		}
		else
		{
			phi[i] = INF;
			state[i] = FAR;
		}
	}
	cout<<endl;
	
	BasicHeap<HeapElem> heap;
	map<long, HeapElem*> elems;
	
	// init heap
	{
		long i = -1, j = -1, k = -1;
		for(long ind = 0; ind < tol_sz; ind++)
		{
			i++; if(i%sz0 == 0){i=0; j++; if(j%sz1==0){j=0; k++;}}
			if(state[ind] == ALIVE)
			{
				for(int kk = 0; kk <= 0; kk++)
				{
					long k2 = k+kk;
					if(k2 < 0 || k2 >= sz2) continue;
					for(int jj = -1; jj <= 1; jj++)
					{
						long j2 = j+jj;
						if(j2 < 0 || j2 >= sz1) continue;
						for(int ii = -1; ii <=1; ii++)
						{
							long i2 = i+ii;
							if(i2 < 0 || i2 >= sz0) continue;
							int offset = ABS(ii) + ABS(jj) + ABS(kk);
							if(offset == 0 || offset > cnn_type) continue;
							long ind2 = k2 * sz01 + j2 * sz0 + i2;
							if(state[ind2] == FAR)
							{
								long min_ind = ind;
								// get minimum Alive point around ind2
								if(phi[min_ind] > 0.0)
								{
									for(int kkk = 0; kkk <= 0; kkk++)
									{
										long k3 = k2 + kkk;
										if(k3 < 0 || k3 >= sz2) continue;
										for(int jjj = -1; jjj <= 1; jjj++)
										{
											long j3 = j2 + jjj;
											if(j3 < 0 || j3 >= sz1) continue;
											for(int iii = -1; iii <= 1; iii++)
											{
												long i3 = i2 + iii;
												if(i3 < 0 || i3 >= sz0) continue;
												int offset2 = ABS(iii) + ABS(jjj) + ABS(kkk);
												if(offset2 == 0 || offset2 > cnn_type) continue;
												long ind3 = k3 * sz01 + j3 * sz0 + i3;
												if(state[ind3] == ALIVE && phi[ind3] < phi[min_ind]) min_ind = ind3;
											}
										}
									}
								}
								// over
								phi[ind2] = phi[min_ind] + inimg1d[ind2];
								state[ind2] = TRIAL;
								parent[ind2] = min_ind;
								HeapElem * elem = new HeapElem(ind2, phi[ind2]);
								heap.insert(elem);
								elems[ind2] = elem;
								bdr_count++;
							}
						}
					}
				}
			}
		}
	}
	
	cout<<"bkg_count = "<<bkg_count<<" ("<<bkg_count/(double)tol_sz<<")"<<endl;
	cout<<"bdr_count = "<<bdr_count<<" ("<<bdr_count/(double)tol_sz<<")"<<endl;
	cout<<"elems.size() = "<<elems.size()<<endl;
	// loop
	int time_counter = bkg_count;
	double process1 = 0;
	while(!heap.empty())
	{
		double process2 = (time_counter++)*100000.0/tol_sz;
		if(process2 - process1 >= 10) {cout<<"\r"<<((int)process2)/1000.0<<"%";cout.flush(); process1 = process2;
			//SAVE_PHI_IMAGE(phi, sz0, sz1, sz2, string("phi") + num2str((int)process1) + ".tif");
		}
		
		HeapElem* min_elem = heap.delete_min();
		elems.erase(min_elem->img_ind);
		
		long min_ind = min_elem->img_ind;
		delete min_elem;
		
		state[min_ind] = ALIVE;
		int i = min_ind % sz0; 
		int j = (min_ind/sz0) % sz1; 
		int k = (min_ind/sz01) % sz2;
		
		int w, h, d;
		for(int kk = 0; kk <= 0; kk++)
		{
			d = k+kk;
			if(d < 0 || d >= sz2) continue;
			for(int jj = -1; jj <= 1; jj++)
			{
				h = j+jj;
				if(h < 0 || h >= sz1) continue;
				for(int ii = -1; ii <= 1; ii++)
				{
					w = i+ii;
					if(w < 0 || w >= sz0) continue;
					int offset = ABS(ii) + ABS(jj) + ABS(kk);
					if(offset == 0 || offset > cnn_type) continue;
					long index = d*sz01 + h*sz0 + w;
					
					if(state[index] != ALIVE)
					{
						float new_dist = phi[min_ind] + inimg1d[index] * sqrt(double(offset));
						
						if(state[index] == FAR)
						{
							phi[index] = new_dist;
							HeapElem * elem = new HeapElem(index, phi[index]);
							heap.insert(elem);
							elems[index] = elem;
							state[index] = TRIAL;
							parent[index] = min_ind;
						}
						else if(state[index] == TRIAL)
						{
							if(phi[index] > new_dist)
							{
								phi[index] = new_dist;
								HeapElem * elem = elems[index];
								heap.adjust(elem->heap_id, phi[index]);
								parent[index] = min_ind;
							}
						}
					}
				}
			}
		}
	}
	
	map<long, MyMarker*> marker_map;
	for(long ind = 0; ind < tol_sz; ind++)
	{
		if(inimg1d[ind] < bkg_thresh || parent[ind] == ind) continue;
		int i = ind % sz0;
		int j = ind / sz0 % sz1;
		int k = ind / sz01;
		MyMarker * marker = new MyMarker(i, j, k);
		outtree.push_back(marker);
		marker_map[ind] = marker;
	}
	for(int m = 0; m < outtree.size(); m++)
	{
		MyMarker * child_marker = outtree[m];
		MyMarker * parent_marker = marker_map[parent[child_marker->ind(sz0, sz01)]];
		child_marker->parent = parent_marker;
	}

	assert(elems.empty());
	if(phi){delete [] phi; phi = 0;}
	if(parent){delete [] parent; parent = 0;}
	if(state) {delete [] state; state = 0;}
	return true;
}
#endif
