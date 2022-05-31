#include <iostream>
#include <math.h>
#include <stdlib.h>
#include<string.h>
#include<msclr\marshal_cppstd.h>
#include <ctime>// include this header 
#include <mpi.h>// include MPI
#pragma once

#using <mscorlib.dll>
#using <System.dll>
#using <System.Drawing.dll>
#using <System.Windows.Forms.dll>
using namespace std;
using namespace msclr::interop;

int* inputImage(int* w, int* h, System::String^ imagePath) //put the size of image in w & h
{
	int* input;


	int OriginalImageWidth, OriginalImageHeight;

	//*********************************************************Read Image and save it to local arrayss*************************	
	//Read Image and save it to local arrayss

	System::Drawing::Bitmap BM(imagePath);

	OriginalImageWidth = BM.Width;
	OriginalImageHeight = BM.Height;
	*w = BM.Width;
	*h = BM.Height;
	int *Red = new int[BM.Height * BM.Width];
	int *Green = new int[BM.Height * BM.Width];
	int *Blue = new int[BM.Height * BM.Width];
	input = new int[BM.Height*BM.Width];
	for (int i = 0; i < BM.Height; i++)
	{
		for (int j = 0; j < BM.Width; j++)
		{
			System::Drawing::Color c = BM.GetPixel(j, i);

			Red[i * BM.Width + j] = c.R;
			Blue[i * BM.Width + j] = c.B;
			Green[i * BM.Width + j] = c.G;

			input[i*BM.Width + j] = ((c.R + c.B + c.G) / 3); //gray scale value equals the average of RGB values

		}

	}
	return input;
}


void createImage(int* image, int width, int height, int index)
{
	System::Drawing::Bitmap MyNewImage(width, height);


	for (int i = 0; i < MyNewImage.Height; i++)
	{
		for (int j = 0; j < MyNewImage.Width; j++)
		{
			//i * OriginalImageWidth + j
			if (image[i*width + j] < 0)
			{
				image[i*width + j] = 0;
			}
			if (image[i*width + j] > 255)
			{
				image[i*width + j] = 255;
			}
			System::Drawing::Color c = System::Drawing::Color::FromArgb(image[i*MyNewImage.Width + j], image[i*MyNewImage.Width + j], image[i*MyNewImage.Width + j]);
			MyNewImage.SetPixel(j, i, c);
		}
	}
	MyNewImage.Save("..//Data//Output//output2NN" + index + ".png");
	cout << "result Image Saved " << index << endl;
}


int main()
{
	////seq
	////for (int i = 0; i < 1; i++)
	////{
	//	int ImageWidth = 4, ImageHeight = 4;

	//	int start_s, stop_s, TotalTime = 0;

	//	System::String^ imagePath;
	//	std::string img;
	//	img = "..//Data//Input//N.png"; //path of img

	//	imagePath = marshal_as<System::String^>(img);
	//	int* imageData = inputImage(&ImageWidth, &ImageHeight, imagePath); //image 1D
	//	int* data = new int[ImageHeight * ImageWidth]; //save img
	//	int filter = 3;
	//	for (size_t i = 0; i < ImageHeight*ImageWidth; i++) //to padding
	//	{
	//		data[i] = imageData[i];
	//	}
	//	start_s = clock();
	//	for (int i = 0; i < ImageHeight - (filter-1); i++) //height
	//	{
	//		for (int j = 0; j < ImageWidth - (filter - 1); j++) //width
	//		{
	//			int sum = 0;
	//			for (int r = 0; r < filter; r++) //filter
	//			{
	//				for (int c = 0; c < filter; c++)
	//				{
	//					sum += imageData[(i + r) * (ImageWidth) + (j + c)]; //current row and increament by filter row & current col and increament by filter col
	//				}
	//			}

	//			//save in center
	//			data[(i * (ImageWidth) + j+1)+ImageWidth] = sum / 9; //row+(col+1)+down 1 row
	//		}
	//	}

	//	stop_s = clock();
	//	TotalTime += (stop_s - start_s) / double(CLOCKS_PER_SEC) * 1000;
	//	cout << "time: " << TotalTime << endl;
	//	createImage(data, ImageWidth, ImageHeight, 0);
	//	free(imageData);
	////}
	//int x;
	//cin >> x;
	//return 0;
	

	//paralel
	MPI_Init(NULL, NULL);
	//for (int i = 0; i < 10; i++)
	//{
		//rank of processor
		int rank;
		MPI_Comm_rank(MPI_COMM_WORLD, &rank);
		//num of processors
		int size;
		MPI_Comm_size(MPI_COMM_WORLD, &size);
		int ImageWidth = 4, ImageHeight = 4;
		int start_s, stop_s, TotalTime = 0;
		System::String^ imagePath;
		std::string img;
		img = "..//Data//Input//2N.png";

		imagePath = marshal_as<System::String^>(img);
		int* imageData = inputImage(&ImageWidth, &ImageHeight, imagePath);
		//int* data = new int[(ImageHeight - 2) * (ImageWidth - 2)];
		start_s = clock();
		//wait until all processors ready
		MPI_Barrier(MPI_COMM_WORLD);
		//how row we will send
		int num_rows_send = (ImageHeight - 2) / size; //exclusive 2end row
		//start ind row i will send
		int start_ind = (num_rows_send * rank);
		//when i stop ind row
		int end_ind = start_ind + num_rows_send;
		//if my rank final processor i do end = imageheight execlusive 2end row to calc reminder
		if (rank == size - 1)
			end_ind = ImageHeight - 2;

		//num/size of index i will apply filter //num of rows*width
		int size_work_now = (end_ind - start_ind) * (ImageWidth - 2);
		//array calc in it data filtered
		int* Data_processor = new int[ImageHeight * ImageWidth];


		for (int i = start_ind, cnt = 0; i < end_ind; i++, cnt++) //height /row
		{
			for (int j = 0; j < ImageWidth - 2; j++) //width /col
			{
				int sum = 0;
				for (int r = 0; r < 3; r++) //filter 
				{
					for (int c = 0; c < 3; c++)
					{
						sum += imageData[(i + r) * (ImageWidth- 2) + (j + c)];//current row and increament by filter row & current col and increament by filter col
					}
				}

				//save in array
				Data_processor[(cnt * (ImageWidth - 2) + j)] = sum / 9; //row+col to save in ordering
			}
		}


		if (rank != 0)
		{
			//MPI_Send(Data_processor, size_work_now, MPI_INT, 0, rank, MPI_COMM_WORLD);
			MPI_Gatherv(Data_processor, size_work_now, MPI_INT, NULL, NULL, NULL, MPI_INT, 0, MPI_COMM_WORLD);
		}
		else
		{
			int* cnts = new int[size];
			int* disp = new int[size];//ind
			for (int i = 0; i < size; i++)
			{
				if (i != size - 1)
				{
					cnts[i] = size_work_now;//size i will send
					//cout << size_work_now<<" ";
				}
				else {
					int cal = ((ImageWidth - 2) * (ImageHeight - 2)) / size_work_now; //20/5 = 3
					cnts[i] = ((ImageWidth - 2) * (ImageHeight - 2)) - (--cal * size_work_now); //20 - 2*5
					//cout <<i<<": ranked    "<< cnts[i] << " : ";
				}
				disp[i] = i * size_work_now;
				//disp[i] = ((i+1) * size_work_now)+1;
			}
			//MPI_Status sts;
			MPI_Gatherv(Data_processor, size_work_now, MPI_INT, imageData, cnts, disp, MPI_INT, 0, MPI_COMM_WORLD);

			/*MPI_Send(Data_processor, size_work_now, MPI_INT, 0, 0, MPI_COMM_WORLD);
			for (int i = 0; i < size; i++)
			{
				MPI_Recv(imageData, cnts[i],MPI_INT,i,i,MPI_COMM_WORLD,&sts);
			}*/
			stop_s = clock();
			TotalTime += (stop_s - start_s) / double(CLOCKS_PER_SEC) * 1000;
			cout << "time: " << TotalTime << endl;
			createImage(imageData, ImageWidth, ImageHeight, 0);
		}

		

		free(imageData);
		// }
		MPI_Finalize();
		return 0;
		
}



