#include "config.h"

#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

#include <tesseract/baseapi.h>
#include <leptonica/allheaders.h>

#include <iostream>
#include <vector>
#include <functional>



namespace {
	constexpr int width = 350;
	constexpr int height = 450;
}

struct HSV{
	int h;
	int s;
	int v;
};;


cv::Mat PreProcessImage(const cv::Mat&);
std::vector<cv::Point> sortContourPoints(const std::vector<cv::Point>& contour);
std::vector<cv::Point> findContours(const cv::Mat&, std::function<int(const std::vector<std::vector<cv::Point>>&)> callback);
void DrawPoints(const std::vector<cv::Point>& points, cv::Mat& img);

int main() {



	cv::Mat img;
	img = cv::imread("../Resource/paper2.jpg");

	if (img.empty()) {
		std::cerr << "Error: Could not load image!\n";
		return -1;
	}

	cv::resize(img, img, cv::Size(), 0.5, 0.5);
	cv::Mat pImg = PreProcessImage(img);

	auto getLargestRectangleContour = [&](const std::vector<std::vector<cv::Point>>& contours){
		int largestIndex = -1;
		cv::drawContours(img, contours, -1, cv::Scalar(255.f, 0.f, 0.f), 3);
		double largestArea = 0.0;
		for (int i = 0; i < contours.size(); ++i) {

			if (contours[i].size() != 4) continue;
			
			double area = cv::contourArea(contours[i]);
			std::cout << area << std::endl;
			if (area > largestArea) {
				largestArea = area;
				largestIndex = i;
			}
		}
		return largestIndex;
	};

	auto& result = findContours(pImg, getLargestRectangleContour);

	if (result.size() > 0) {
		std::vector<std::vector<cv::Point>> contoursVec = { result };



		result = sortContourPoints(result);
		std::vector<cv::Point2f> srcPoints;
		for (auto& p : result) {
			srcPoints.push_back(cv::Point2f(p.x, p.y));
		}



		std::vector<cv::Point2f> dest = {
		{0.f, 0.f},
		{width, 0.f},
		{0.f, height},
		{width, height}
		};

		cv::Mat transformedMatrix = cv::getPerspectiveTransform(srcPoints, dest);
		cv::Mat warpedImg;
		cv::warpPerspective(img, warpedImg, transformedMatrix, cv::Size(width, height));

		{
			tesseract::TessBaseAPI tess;
			if (tess.Init("../Resource/tessdata_fast-main", "eng")) {  // nullptr = use default tessdata path, "eng" = English
				std::cerr << "Could not initialize tesseract.\n";
				return -1;
			}

			cv::Mat gray;
			cv::cvtColor(warpedImg, gray, cv::COLOR_BGR2GRAY);
			cv::threshold(gray, gray, 128, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);

			tess.SetImage(gray.data, gray.cols, gray.rows, gray.channels(), gray.step);


			tess.Recognize(0);
			tesseract::ResultIterator* ri = tess.GetIterator();
			tesseract::PageIteratorLevel level = tesseract::RIL_WORD;

			if (ri != nullptr) {
				do {
					const char* word = ri->GetUTF8Text(level);
					float conf = ri->Confidence(level);
					int x1, y1, x2, y2;
					ri->BoundingBox(level, &x1, &y1, &x2, &y2);
					std::cout << "Word: " << word << " [" << x1 << "," << y1 << "," << x2 << "," << y2
						<< "] Conf: " << conf << "\n";
					delete[] word;
				} while (ri->Next(level));
			}

		}



		cv::imshow("Warped window", warpedImg);
		cv::drawContours(img, contoursVec, 0, cv::Scalar(255.f, 0.f, 0.f), 3);
		DrawPoints(result, img);
	}

	cv::imshow("Display window", img);
	





	cv::waitKey(0);

	
	return 0;
}

cv::Mat PreProcessImage(const cv::Mat& img) {
	cv::Mat gray, blur, edges;

	// 1. Convert to grayscale
	cv::cvtColor(img, gray, cv::COLOR_BGR2GRAY);

	// 2. Reduce noise (Gaussian blur or bilateral)
	cv::GaussianBlur(gray, blur, cv::Size(5, 3), 0);
	//cv::bilateralFilter(gray, blur, 9, 75, 75);

	// 3. Adaptive threshold or Canny edge
	cv::Mat edges1, edges2;
	cv::Canny(blur, edges1, 50, 150);

	// 4. Optional: Morphological closing to fill gaps
	cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3));
	cv::morphologyEx(edges1, edges2, cv::MORPH_CLOSE, kernel);

	return edges2;
}

std::vector<cv::Point> sortContourPoints(const std::vector<cv::Point>& contour) {
	std::vector<cv::Point> pts;

	if (contour.size() <= 0) return pts;

	for (auto& p : contour) pts.push_back(cv::Point(p.x, p.y));

	std::vector<cv::Point> top, bottom;
	std::sort(pts.begin(), pts.end(), [](const cv::Point& a, const cv::Point& b) { return a.y < b.y; });

	top.push_back(pts[0]);
	top.push_back(pts[1]);
	bottom.push_back(pts[2]);
	bottom.push_back(pts[3]);

	std::sort(top.begin(), top.end(), [](const cv::Point& a, const cv::Point& b) { return a.x < b.x; });
	std::sort(bottom.begin(), bottom.end(), [](const cv::Point& a, const cv::Point& b) { return a.x < b.x; });

	return { top[0], top[1], bottom[0], bottom[1] }; // tl, tr, bl, br
}

std::vector<cv::Point> findContours(const cv::Mat& image, std::function<int(const std::vector<std::vector<cv::Point>>&)> callback) {
	std::vector<std::vector<cv::Point>> contours;
	std::vector<std::vector<cv::Point>> contoursPolygon;

	std::vector<cv::Point> result;
	std::vector<cv::Vec4i> hierarchy;
	cv::findContours(image, contours,hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

	int n{};
	contoursPolygon.resize(contours.size());
	for (const auto& contour : contours) {
		float peri = cv::arcLength(contour, true);
		cv::approxPolyDP(contour, contoursPolygon[n], 0.02 * peri, true);
		n++;
	}




	int index = callback(contoursPolygon);

	if (index >= 0 && index < contoursPolygon.size()) {
		return contoursPolygon[index];
	}


	return result;
}



void DrawPoints(const std::vector<cv::Point>& points, cv::Mat& img) {
	
	for (const auto& point : points) {
		cv::circle(img, point, 5, cv::Scalar(0.f, 0.f, 255.f), 3);
	}
	

}