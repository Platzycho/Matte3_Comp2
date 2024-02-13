#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <Eigen/Dense>
#include <iostream>
#include <vector>
#include <algorithm>
#include <utility>
#include <string>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <cmath>
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "Shader.h"
#include "Camera.h"
#include "CoordinateIteration.h"

struct CallbackData {
    Shader* myShader;
    Camera* myCamera;
};

void framebuffer_size_callback(GLFWwindow* window, int width, int height);

void processInput(GLFWwindow* window);

void mouse_callback(GLFWwindow* window, double xpos, double ypos);

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

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

std::vector<std::pair<double, double>> calculateParabolaPoints(double a, double b, double c, double xStart, double xEnd, double xIncrement);

std::string formatParabolaEquation(double a, double b, double c);

Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));

float deltaTime = 0.0f;
float lastFrame = 0.0f;
float rotationAngle = 0.0f;



int main() {

	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(800, 600, "LearnOpenGL", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to init GLAD" << std::endl;
		return -1;
	}

	glViewport(0, 0, 800, 600);

	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    std::vector<float> vertices;
	

    Shader myShader("shader.vs", "shader.fs");

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

	std::vector<std::pair<double, double>> parabolaPoints = calculateParabolaPoints(coeffs[0], coeffs[1], coeffs[2], -10, 10, 1);

	std::ofstream outFile("parabola_points.txt");
	if (!outFile) {
        std::cerr << "Error opening file for writing.\n";
        return 1;
    }
	
	outFile << "\nThe parabola equation for this matrix is:\n" << equation << "\n";

	std::cout << "Calculated points on the parabola:\n";
    for (const auto& point : parabolaPoints) {
        std::cout << "(" << point.first << ", " << point.second << ")\n";
		float x = static_cast<float>(point.first);
		float y = static_cast<float>(point.second);

		vertices.push_back(x);
		vertices.push_back(y);
    }

	outFile << "Calculated points on the parabola:\n";
    for (const auto& point : parabolaPoints) {
        outFile << "(" << point.first << ", " << point.second << ")\n";
    }

	outFile.close();

	glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)800 / (float)600, 0.1f, 100.0f);
	glm::mat4 view = camera.GetViewMatrix();

    CallbackData callbackData;
    callbackData.myShader = &myShader;
    callbackData.myCamera = &camera;

	unsigned int VBO, VAO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0); 
	glBindVertexArray(0);

    glfwSetScrollCallback(window, scroll_callback);

    glfwSetWindowUserPointer(window, &callbackData);

    glfwSetCursorPosCallback(window, mouse_callback);

    glEnable(GL_DEPTH_TEST);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

		while(!glfwWindowShouldClose(window))
	{
		float currentFrame = static_cast<float>(glfwGetTime());
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;
		processInput(window);

		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		myShader.use();
        
	    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)800 / (float)600, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();

        glm::mat4 model = glm::mat4(1.0f);
        glUniformMatrix4fv(glGetUniformLocation(myShader.ID, "model"), 1, GL_FALSE, glm::value_ptr(model));

        myShader.setMat4("projection", projection);
        myShader.setMat4("view", view);

		glBindVertexArray(VAO);
		
		glDrawArrays(GL_LINE_STRIP, 0, vertices.size() / 2);

		glPointSize(5.0f);
		glDrawArrays(GL_POINTS, 0, vertices.size() / 2);

        glfwSwapBuffers(window);
        glfwPollEvents();	
	}

    glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glfwTerminate();
	return 0;

	return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
    
    CallbackData* data = static_cast<CallbackData*>(glfwGetWindowUserPointer(window));

    // Now you can use data->myShader and data->camera as needed
     if (data != nullptr && data->myShader != nullptr) {
        glm::mat4 projection = glm::perspective(glm::radians(data->myCamera->Zoom), (float)width / (float)height, 0.1f, 100.0f);
        data->myShader->use(); // Make sure the shader is being used before setting uniforms
        glUniformMatrix4fv(glGetUniformLocation(data->myShader->ID, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    }
}

void processInput(GLFWwindow* window)
{	
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        camera.ProcessKeyboard(UP, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        camera.ProcessKeyboard(DOWN, deltaTime);

	if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    static float lastX = 800.0f / 2.0;
    static float lastY = 600.0f / 2.0;
    static bool firstMouse = true;

    CallbackData* data = static_cast<CallbackData*>(glfwGetWindowUserPointer(window));
    if(firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    if (data && data->myCamera) {
        data->myCamera->ProcessMouseMovement(xoffset, yoffset);
    }
    

}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    CallbackData* data = static_cast<CallbackData*>(glfwGetWindowUserPointer(window));
    if(data && data->myCamera) {
        data->myCamera->ProcessMouseScroll(yoffset);
    }
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

std::vector<std::pair<double, double>> calculateParabolaPoints(double a, double b, double c, double xStart, double xEnd, double xIncrement)
{
	std::vector<std::pair<double, double>> points;

	for(double x = xStart; x <= xEnd; x += xIncrement){
		double y = a * x * x + b * x + c;
		points.emplace_back(x, y);
	}
	return points;
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

