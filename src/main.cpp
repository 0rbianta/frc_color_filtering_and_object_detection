#include <iostream>
#include <string>
#include <opencv2/opencv.hpp>
#include "include/fs.h"
#include <vector>
#include <regex>


using namespace std;
using namespace cv;

typedef struct
{
	string name;
	Scalar hsv_low;
	Scalar hsv_high;
} hsv_range;

void replace_str(string &str, string search, string replace)
{
	size_t i = str.find(search);
	str.replace(i, search.length(), replace);
}


void parse_color_cfg(string content, vector<hsv_range> &hsv_ranges_out, string &selection_out)
{
	string line = "";
	vector<string> lines;
	for (int i = 0; i < content.length(); i++)
	{
		char pch = content[i];
		if (pch == '\n')
		{
			lines.push_back(line);
			line = "";
		}
		else
			line += pch;

	}

	string SELECTION;
	bool reading_seg_data = false;
	hsv_range p_hsv_range;
	vector<hsv_range> hsv_ranges;

	for (int i = 0; i < lines.size(); i++)
	{
		line = lines[i];

		if (i == 0 && line != "!FRC_COLOR_CONF")
		{
			cout << "Invalid color config. Meta info not found." << endl;
			exit(1);
		}


		if (regex_match(line, regex("SELECTION=[a-zA-Z_]+")))
		{
			replace_str(line, "SELECTION=", "");
			SELECTION = line;
		}

		if (line == "%DATA_START%")
			reading_seg_data = true;
		if (line == "%DATA_END%")
		{
			if (!p_hsv_range.name.empty())
				hsv_ranges.push_back(p_hsv_range);
			reading_seg_data = false;
		}


		if (reading_seg_data)
		{
			if (regex_match(line, regex("[a-zA-Z_]+:")))
			{
				if (!p_hsv_range.name.empty())
					hsv_ranges.push_back(p_hsv_range);

				replace_str(line, ":", "");
				p_hsv_range.name = line;
			}
			if (regex_match(line, regex("low_h=[0-9]+")))
			{
				replace_str(line, "low_h=", "");
				p_hsv_range.hsv_low[0] = stoi(line);
			}
			if (regex_match(line, regex("low_s=[0-9]+")))
			{
				replace_str(line, "low_s=", "");
				p_hsv_range.hsv_low[1] = stoi(line);
			}
			if (regex_match(line, regex("low_v=[0-9]+")))
			{
				replace_str(line, "low_v=", "");
				p_hsv_range.hsv_low[2] = stoi(line);
			}
			if (regex_match(line, regex("high_h=[0-9]+")))
			{
				replace_str(line, "high_h=", "");
				p_hsv_range.hsv_high[0] = stoi(line);
			}
			if (regex_match(line, regex("high_s=[0-9]+")))
			{
				replace_str(line, "high_s=", "");
				p_hsv_range.hsv_high[1] = stoi(line);
			}
			if (regex_match(line, regex("high_v=[0-9]+")))
			{
				replace_str(line, "high_v=", "");
				p_hsv_range.hsv_high[2] = stoi(line);
			}
		}
		
	}

	if (reading_seg_data)
	{
		cout << "%DATA_START% tag did not closed." << endl;
		exit(1);
	}
	
	if (SELECTION.empty())
	{
		cout << "Selection is empty" << endl;
		exit(1);
	}
	
	if (hsv_ranges.size() <= 0)
	{
		cout << "No HSV data detected." << endl;
		exit(1);
	}

	bool is_selection_data_exist = false;
	for (int i = 0; i < hsv_ranges.size(); i++)
	{
		if (hsv_ranges[i].name == SELECTION)
			is_selection_data_exist = true;
	}
	
	if (!is_selection_data_exist)
	{
		cout << "Selected color not defined in data segment" << endl;
		exit(1);
	}

	selection_out = SELECTION;
	hsv_ranges_out = hsv_ranges;
}


int main(int argc, char **argv)
{

	string img_path;
	string color_cfg;


	for (int i = 0; i < argc; i++)
	{

		if (string(argv[i]) == "-ccfg" && argc > i + 1)
		{
			color_cfg = argv[i + 1];
		}

		if (string(argv[i]) == "-i" && argc > i + 1)
		{
			img_path = argv[i + 1];
		}
		
	}

	if (color_cfg.empty())
	{
		cout << "Color configuration not defined. Please define configuration by using -ccfg flag." << endl;
		exit(1);
	}

	if (!img_path.empty())
	{
		Mat im = imread(img_path);
		if (im.empty())
		{
			cout << "Failed to read the input image file" << endl;
			exit(1);
		}

		string ccfg_content = read_file(color_cfg);
		vector<hsv_range> hsv_ranges;
		string selection; 
		parse_color_cfg(ccfg_content, hsv_ranges, selection);
		hsv_range selected_hsv_range;
		for (int i = 0; i < hsv_ranges.size(); i++)
			if (hsv_ranges[i].name == selection)
			{
				selected_hsv_range = hsv_ranges[i];
				break;
			}
		

		// Finally time to do the detection
		// Change color space to HSV(Hue, Saturation, Value)
		cvtColor(im, im, COLOR_BGR2HSV);
		// Blur image to make better detection and avoid from gaps on output
		medianBlur(im, im, 71);

		// Only get certain areas that contains pixel values in the range
		inRange(im, selected_hsv_range.hsv_low, selected_hsv_range.hsv_high, im);
		// Make the black thinner and white thicker
		dilate(im, im, Mat(), Point(-1, -1), 3, 1, 1);
		
		imshow("Output", im);
		waitKey(0);
		destroyAllWindows();

	}


	return 0;
}

