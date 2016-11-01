// Noah Rubin

#define SHOW_COMMAND_PROMPT 1

#ifdef _WIN32
#	include "resource.h"
#	if !SHOW_COMMAND_PROMPT
#		include <Windows.h>
#	endif
#endif

#include <random>
#include <chrono>
#include <thread>
#include <atomic>
#include <iostream>

#include "Libraries/gl/glew.h"
#include "Libraries/gl/glfw3.h"
#include "Libraries/glm/glm.hpp"
#include "Libraries/glm/ext.hpp"

#include "Board.h"
#include "ResourceLoader.h"
#include "AI.h"


namespace
{
	constexpr float RADIUS_MAGIC  = 0.085f;
	const glm::vec4 backgroundColor(0.25f, 0.43f, 0.5f, 1.0f);

	glm::mat4 projection;
	glm::mat4 modelView = glm::lookAt(glm::vec3(0.0f, 0.0f, -6.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	GLuint boardProgram;
	GLuint chipProgram;
	GLuint loadingProgram;
	int hoveredColumn = -1;
	bool gameOver = false;
	std::atomic<bool> inputDisabled = false;
	std::atomic<int> fillRow = -1;
	std::atomic<int> fillCol = -1;
	Board board;
	AI ai;


	template<class T>
	bool InRange(T x, T min, T max)
	{
		return x >= min && x <= max;
	}


	void Resize(GLFWwindow* win, int w, int h)
	{
		glViewport(0, 0, w, h);
		projection = glm::perspective(90.0f, (float) w / h, 1.0f, 100.0f);
		glm::mat4 mvp = projection * modelView;
		glUniformMatrix4fv(glGetUniformLocation(boardProgram, "mvpMat"), 1, GL_FALSE, glm::value_ptr(mvp));
		glUniformMatrix4fv(glGetUniformLocation(chipProgram, "mvp"), 1, GL_FALSE, glm::value_ptr(mvp));
		glUniformMatrix4fv(glGetUniformLocation(loadingProgram, "mvp"), 1, GL_FALSE, glm::value_ptr(mvp));
	}


	void SetColorUniform(int col, int row, Chip chip)
	{
		glm::vec4 color = (chip == CHIP_BLACK ? glm::vec4(0.0f, 0.0f, 0.0f, 1.0f) : glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
		GLint unif = glGetUniformLocation(chipProgram, "colors") + Board::WIDTH * row + col;
		glUseProgram(chipProgram);
		glUniform4fv(unif, 1, glm::value_ptr(color));
	}


	void Startup()
	{
		glm::vec4 colors[42];
		for (int i = 0; i < 42; ++i) {
			colors[i] = backgroundColor;
		}
		glUseProgram(chipProgram);
		glUniform4fv(glGetUniformLocation(chipProgram, "colors"), 42, (float*) colors);

		char in = 0;
		while (in != '0' && in != '1') {
			std::cout << "Enter 1 for human first, 0 for ai first\n";
			std::cin >> in;
		}
		if (in == '0') {
			int col = ai.BestMove(board);
			int row = board.Drop(col);
			SetColorUniform(col, row, board.GetNextTurn());
		}
	}


	void CheckWinner()
	{
		if (board.GetWinner() == CHIP_BLACK) {
			std::cout << "Black wins\n";
		}
		else if (board.GetWinner() == CHIP_RED) {
			std::cout << "Red wins\n";
		}
	}


	void Click(GLFWwindow* win, int button, int action, int mods)
	{
		if (action == GLFW_PRESS && button == GLFW_MOUSE_BUTTON_1 && hoveredColumn != -1 && !board.IsColumnFull(hoveredColumn) && !inputDisabled) {
			int row = board.Drop(hoveredColumn);
			SetColorUniform(hoveredColumn, row, board.GetNextTurn());
			CheckWinner();
			if (!board.IsBoardFull()) {
				inputDisabled = true;				
				std::thread([]()
				{
					fillCol = ai.BestMove(board);
					fillRow = board.Drop(fillCol);
					CheckWinner();
					if (board.GetWinner() != CHIP_NONE || board.IsBoardFull()) {
						inputDisabled = true;
					}
					else {
						inputDisabled = false;
					}
				}).detach();
			}
			else {
				std::cout << "Draw\n";
			}
		}
	}


	void MouseMoved(GLFWwindow* win, double x, double y)
	{
		int w, h;
		float z;
		glfwGetWindowSize(win, &w, &h);
		glReadPixels(x, y, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &z);
		z = 0.0f;
		glm::vec3 pos = glm::unProject(
			glm::vec3(x, h - y, z),
			modelView,
			projection,
			glm::vec4(0.0f, 0.0f, w, h)
			);
		hoveredColumn = -1;
		if (InRange(pos.x, -0.82f - RADIUS_MAGIC, -0.82f + RADIUS_MAGIC)) {
			hoveredColumn = 6;
		}
		else if (InRange(pos.x, -0.55f - RADIUS_MAGIC, -0.55f + RADIUS_MAGIC)) {
			hoveredColumn = 5;
		}
		else if (InRange(pos.x, -0.28f - RADIUS_MAGIC, -0.28f + RADIUS_MAGIC)) {
			hoveredColumn = 4;
		}
		else if (InRange(pos.x, -0.01f - RADIUS_MAGIC, -0.01f + RADIUS_MAGIC)) {
			hoveredColumn = 3;
		}
		else if (InRange(pos.x, 0.28f - RADIUS_MAGIC, 0.28f + RADIUS_MAGIC)) {
			hoveredColumn = 2;
		}
		else if (InRange(pos.x, 0.55f - RADIUS_MAGIC, 0.55f + RADIUS_MAGIC)) {
			hoveredColumn = 1;
		}
		else if (InRange(pos.x, 0.82f - RADIUS_MAGIC, 0.82f + RADIUS_MAGIC)) {
			hoveredColumn = 0;
		}
	}


	GLFWwindow* InitWindow(int width, int height, const char* title)
	{
		if (!glfwInit()) {
			return nullptr;
		}
		glfwWindowHint(GLFW_SAMPLES, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		GLFWwindow* win = glfwCreateWindow(width, height, title, nullptr, nullptr);
		if (!win) {
			return nullptr;
		}
		glfwMakeContextCurrent(win);
		glewExperimental = true;
		if (glewInit() != GLEW_OK) {
			return nullptr;
		}
		glfwSetWindowSizeCallback(win, &Resize);
		glfwSetMouseButtonCallback(win, &Click);
		glfwSetCursorPosCallback(win, &MouseMoved);
		return win;
	}


	std::vector<AttributeData> CircleVerts()
	{
		static constexpr int numTriangles = 100;
		float increment = 2.0f * glm::pi<float>() / ((numTriangles + 2) / 3);
		std::vector<AttributeData> verts((numTriangles + 2) * 3);
		float currentAngle = 2.0f * glm::pi<float>();
		verts[0].fVal = 0.0f;
		verts[1].fVal = 0.0f;
		verts[2].fVal = -0.51f;
		for (int i = 3; i < verts.size() - 1; i += 3) {
			verts[i].fVal = 0.5f * std::cos(currentAngle);
			verts[i + 1].fVal = 0.5f * std::sin(currentAngle);
			verts[i + 2].fVal = -0.51f;
			currentAngle -= increment;
		}
		return verts;
	}


	void DoAITest()
	{
		std::getchar();
		int col = ai.BestMove(board);
		int row = board.Drop(col);
		SetColorUniform(col, row, board.GetNextTurn());
		CheckWinner();
		if (!board.IsBoardFull()) {
			col = ai.BestMove(board);
			row = board.Drop(col);
			SetColorUniform(col, row, board.GetNextTurn());
			CheckWinner();
		}
		else {
			std::cout << "Draw\n";
		}
	}


	void LoadShaders(GLFWwindow* win)
	{
#ifdef _WIN32
		GLuint boardVertShader = LoadShaderFile(BOARD_VERT_SHADER, GL_VERTEX_SHADER);
		GLuint boardFragShader = LoadShaderFile(BOARD_FRAG_SHADER, GL_FRAGMENT_SHADER);
		GLuint chipVertShader = LoadShaderFile(CHIP_VERT_SHADER, GL_VERTEX_SHADER);
		GLuint chipFragShader = LoadShaderFile(CHIP_FRAG_SHADER, GL_FRAGMENT_SHADER);
		GLuint loadingVertShader = LoadShaderFile(LOADING_VERT_SHADER, GL_VERTEX_SHADER);
		GLuint loadingFragShader = LoadShaderFile(LOADING_FRAG_SHADER, GL_FRAGMENT_SHADER);
#else
		GLuint boardVertShader = LoadShaderFile("board.vert", GL_VERTEX_SHADER);
		GLuint boardFragShader = LoadShaderFile("board.frag", GL_FRAGMENT_SHADER);
		GLuint chipVertShader = LoadShaderFile("chip.vert", GL_VERTEX_SHADER);
		GLuint chipFragShader = LoadShaderFile("chip.frag", GL_FRAGMENT_SHADER);
		GLuint loadingVertShader = LoadShaderFile("loading.vert", GL_VERTEX_SHADER);
		GLuint loadingFragShader = LoadShaderFile("loading.vert", GL_FRAGMENT_SHADER);
#endif
		boardProgram = LinkShaders(boardVertShader, boardFragShader);
		chipProgram = LinkShaders(chipVertShader, chipFragShader);
		loadingProgram = LinkShaders(loadingVertShader, loadingFragShader);
		glUseProgram(boardProgram);
		Resize(win, 600, 400);
		glUseProgram(chipProgram);
		Resize(win, 600, 400);
		glUseProgram(loadingProgram);
		Resize(win, 600, 400);
	}


	InstancedVertexMesh CreateCircleMesh()
	{
		std::vector<AttributeData> verts = CircleVerts();
		VertexAttribute attribs(verts, 0, 3, GL_FLOAT);
		return InstancedVertexMesh({ attribs }, GL_TRIANGLE_FAN, 42);
	}

	VertexMesh CreateLoadingMesh()
	{		
		static constexpr int numTriangles = 100;
		float increment = 2.0f * glm::pi<float>() / ((numTriangles + 2) / 3);
		std::vector<AttributeData> verts(numTriangles * 9);
		float currentAngle = 2.0f * glm::pi<float>();
		for (int i = 0; i < verts.size(); i += 9) {
			verts[i].fVal = 0.0f;
			verts[i + 1].fVal = 0.0f;
			verts[i + 2].fVal = -0.51f;

			verts[i + 3].fVal = 0.5f * std::cos(currentAngle);
			verts[i + 4].fVal = 0.5f * std::sin(currentAngle);
			verts[i + 5].fVal = -0.51f;
			currentAngle -= increment;

			verts[i + 6].fVal = 0.5f * std::cos(currentAngle);
			verts[i + 7].fVal = 0.5f * std::sin(currentAngle);
			verts[i + 8].fVal = -0.51f;

			currentAngle -= increment;
		}

		int size = verts.size();

		VertexAttribute attribs(verts, 0, 3, GL_FLOAT);
		return VertexMesh({ attribs }, GL_TRIANGLES);
	}


	IndexedMesh CreateBoardMesh()
	{
		GLfloat boardVerts[]{
			-5.5f, 4.75f, -0.5f,	// top left
			5.5f, 4.75f, -0.5f,		// top right
			-5.5f, -4.75f, -0.5f,	// bottom left
			5.5f, -4.75f, -0.5f		// bottom right
		};
		std::vector<GLushort> boardIndices{
			0, 1, 2,
			2, 1, 3
		};
		std::vector<VertexAttribute> boardAttribs{ VertexAttribute(0, 3, GL_FLOAT) };
		boardAttribs[0].AddData(boardVerts, 12);
		return IndexedMesh(boardAttribs, boardIndices, GL_TRIANGLES);
	}

	void RotateLoadingColors()
	{
		static double lastTime = -1.0;
		static glm::vec4 colors[] {
			colors[0] = glm::vec4{ 0.6f, 0.6f, 0.6f , 1.0f },
			colors[1] = glm::vec4{ 0.75f, 0.75f, 0.75f , 1.0f },
			colors[2] = glm::vec4{ 0.9f, 0.9f, 0.9f , 1.0f },
		};

		if (glfwGetTime() - lastTime > 1.0) {
			glUniform4fv(glGetUniformLocation(loadingProgram, "colors"), 3, (float*) colors);
			std::swap(colors[0], colors[1]);
			std::swap(colors[0], colors[2]);
			lastTime = glfwGetTime();
		}
	}
}


int main()
{
#if defined(_WIN32) && !SHOW_COMMAND_PROMPT
	FreeConsole();
#endif
	GLFWwindow* win = InitWindow(600, 400, "Connect Four by Noah Rubin");
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glDepthMask(GL_TRUE);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);

	glGetError();

	LoadShaders(win);

	glm::vec2 translations[42];
	for (int i = 0; i < 42; ++i) {
		int row = i / Board::WIDTH;
		int col = i % Board::WIDTH;
		translations[i].x = 4.5f - 1.5f * col;
		translations[i].y = 3.75f - 1.5f * row;
	}

	glUseProgram(chipProgram);
	glUniform2fv(glGetUniformLocation(chipProgram, "translations"), 42, (float*) translations);

	Startup();

	IndexedMesh boardMesh = CreateBoardMesh();
	InstancedVertexMesh circleMesh = CreateCircleMesh();
	VertexMesh loadingMesh = CreateLoadingMesh();
	while (!glfwWindowShouldClose(win)) {
		if (fillCol != -1) {
			SetColorUniform(fillCol, fillRow, board.GetNextTurn());
			fillCol = -1;
			fillRow = -1;
		}
		glClearColor(backgroundColor.r, backgroundColor.g, backgroundColor.b, backgroundColor.a);
		glClearDepth(1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glUseProgram(boardProgram);
		boardMesh.Render();
		glUseProgram(chipProgram);
		circleMesh.Render();
		if (inputDisabled) {
			glUseProgram(loadingProgram);
			RotateLoadingColors();
			loadingMesh.Render();
		}

		glfwSwapBuffers(win);
		glfwPollEvents();
		if (gameOver) {
			std::getchar();
			board = Board();
			ai.Reset();
			Startup();
			gameOver = false;
			inputDisabled = false;
		}
		if (board.GetWinner() != CHIP_NONE || board.IsBoardFull()) {
			gameOver = true;
			inputDisabled = true;
		}
	}
	glUseProgram(0);
	glDeleteProgram(boardProgram);
	glDeleteProgram(chipProgram);
	glfwDestroyWindow(win);
	glfwTerminate();
	return 0;
}
