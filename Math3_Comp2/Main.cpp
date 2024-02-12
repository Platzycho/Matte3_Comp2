#include <Eigen/Dense>
#include <iostream>
#include <vector>
#include <algorithm>
#include <utility>
#include <string>
#include <sstream>
#include <iomanip>
#include "CoordinateIteration.h"

std::vector<std::pair<double, double>> pointsOnThePlane = {
	{2, 2},
	{2, 4},
	{4, 2},
	{4, 4},
	{3, 4.5},
	{6, 2},
	{6, 4}
};

std::vector<std::pair<double, double>> coordinates = {
		{2, 2}, // Point 1
		{3, 4.5}, //Point 2
		{6, 4} //Point 3
	};

void addNewPoint(double x, double y);

void removePointByIndex(std::vector<std::pair<double, double>> coordinates, size_t index);

double triangleArea(const std::pair<double, double>& p1, const std::pair<double, double>& p2, const std::pair<double, double>& p3);

std::vector<std::pair<double, double>> bestTriangle(const std::vector<std::pair<double, double>>& coordinates);

Eigen::MatrixXd addCoordinatesToMatrix(std::vector<std::pair<double, double>>& coordinates);

Eigen::MatrixXd createMatrix(std::vector<std::pair<double, double>>& coordinates);

Eigen::MatrixXd invertedMatrix(const Eigen::MatrixXd& matrix);

Eigen::Vector3d findParabola(const Eigen::MatrixXd& coordinates);

std::string formatParabolaEquation(double a, double b, double c);



int main() {

	Eigen::MatrixXd startPoints = addCoordinatesToMatrix(pointsOnThePlane);

	std::cout << "For the first task I chose these points:\n" << startPoints << ".\n" << std::endl;

	auto bestCoords = bestTriangle(coordinates);

	std::cout << "these are the chosen coordinates for our matrix:" << std::endl;

	CoordinateIteration iter(bestCoords);
	while (auto coords = iter.getNext()) {
		std::cout << " (" << coords->first << ", " << coords->second << ")" << std::endl;
	}
	
	Eigen::MatrixXd matrix = addCoordinatesToMatrix(bestCoords);
	std::cout << "\nStart matrix:\n" << matrix << std::endl;

	Eigen::MatrixXd newMatrix = createMatrix(bestCoords);
	std::cout << "\nMatrix after parabolic equation:\n" << newMatrix << std::endl;

	Eigen::MatrixXd invertMatrix = invertedMatrix(matrix);
	std::cout << "\nInverted matrix:\n" << invertMatrix << std::endl;

	Eigen::Vector3d coeffs = findParabola(matrix);
	std::cout << "\nThe parabola coefficients are:\n";
    std::cout << "a: " << coeffs[0] << ", b: " << coeffs[1] << ", c: " << coeffs[2] << std::endl;

	std::string equation = formatParabolaEquation(coeffs[0], coeffs[1], coeffs[2]);
	std::cout << "\nThe parabola equation for this matrix is:\n"<< equation << std::endl;

	return 0;
}

void addNewPoint(double x, double y)
{
	coordinates.push_back({x, y});
}

void removePointByIndex(std::vector<std::pair<double, double>> coordinates, size_t index)
{
	if(index < coordinates.size()){
		coordinates.erase(coordinates.begin() + index);
	}
}

double triangleArea(const std::pair<double, double>& p1, const std::pair<double, double>& p2, const std::pair<double, double>& p3)
{
	double area = 0.5 * std::abs(p1.first * (p2.second - p3.second) + p2.first * (p3.second - p1.second) + p3.first * (p1.second - p2.second)); 

	return area;
}

std::vector<std::pair<double, double>> bestTriangle(const std::vector<std::pair<double, double>>& coordinates)
{
	double maxArea = 0;
	std::vector<std::pair<double, double>> bestCoords(3);

	for(size_t i = 0; i < coordinates.size(); ++i){
		for(size_t j = 0; j < coordinates.size(); ++j){
			for(size_t k = 0; k < coordinates.size(); ++k){
				double area = triangleArea(coordinates[i], coordinates[j], coordinates[k]);
				if (area > maxArea) {
					maxArea = area;
					bestCoords = {coordinates[i], coordinates[j], coordinates[k]};
				
				}
			}
		}
	}


	return bestCoords;
}

Eigen::MatrixXd addCoordinatesToMatrix(std::vector<std::pair<double, double>>& coordinates)
{
	Eigen::MatrixXd matrix(coordinates.size(), 3);

	for(size_t i = 0; i < coordinates.size(); ++i){
		matrix(i, 0) = coordinates[i].first;
		matrix(i, 1) = coordinates[i].second;
		matrix(i, 2) = 1;
	}

	return matrix;
}

Eigen::MatrixXd createMatrix(std::vector<std::pair<double, double>>& coordinates)
{
	Eigen::MatrixXd A(coordinates.size(), 3);

	for(int i = 0; i < coordinates.size(); ++i) {
		double x = coordinates[i].first;
		double y = coordinates[i].second;
		A(i, 0) = x * x;
		A(i, 1) = x;
		A(i, 2) = 1;
	
	}



	return A;
}

Eigen::MatrixXd invertedMatrix(const Eigen::MatrixXd& matrix)
{

	if(matrix.rows() != matrix.cols()){
		std::cerr << "Error: Matrix must be square to compute its inverse." << std::endl;
		return Eigen::MatrixXd();
	}

	Eigen::MatrixXd inverse = matrix.inverse();

	return inverse;
}

Eigen::Vector3d findParabola(const Eigen::MatrixXd& coordinates)
{
	int n = coordinates.rows();
	Eigen::MatrixXd A(n, 3);
	Eigen::VectorXd b(n);

	for (size_t i = 0; i < n; ++i) {
		double x = coordinates(i, 0);
		double y = coordinates(i, 1);
		A(i, 0) = x * x;
		A(i, 1) = x;
		A(i, 2) = 1;
		b(i) = y;
	}

	Eigen::Vector3d coeffs = A.colPivHouseholderQr().solve(b);

	return coeffs;
}

std::string formatParabolaEquation(double a, double b, double c)
{
	std::stringstream ss;
    ss << std::fixed << std::setprecision(2); // Set precision for coefficients

    ss << "y = ";
    if (a != 0) {
        ss << a << "x^2 ";
    }
    if (b != 0) {
        if (b > 0 && a != 0) ss << "+ "; // Add plus sign if b is positive and a is not zero
        ss << b << "x ";
    }
    if (c != 0) {
        if (c > 0 && (a != 0 || b != 0)) ss << "+ "; // Add plus sign if c is positive and either a or b is not zero
        ss << c;
    }
    if (a == 0 && b == 0 && c == 0) {
        ss << "0";
    }

    return ss.str();
}

