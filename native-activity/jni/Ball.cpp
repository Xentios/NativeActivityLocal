/*
 * openglHelper.cpp
 *
 *  Created on: 28 Mar 2013
 *      Author: Engin Manap
 */


#include "Ball.h"


#define PI 3.14159265

#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "native-activity", __VA_ARGS__))

namespace androng {

	void Ball::initializeVertexShader() {
		VSbasic =
				"attribute vec4 vPosition;\n"
				"uniform vec2 offset;\n"
				"uniform mat4 perspectiveMatrix;\n"
				"void main()\n"
				"{\n"
				"vec4 lastPosition = vPosition + vec4(offset.x, offset.y, 0.0, 0.0);\n"
				"gl_Position = perspectiveMatrix * lastPosition;\n"
				"}\n";

	}

	void Ball::initializeFragmentShader() {
		FSbasic =
				"precision mediump float;\n"
				// "uniform vec4 vColor;\n"
				"void main() {\n"
					"vec4 originalColor = vec4 ( 0.5, 0.5, 0.5, 1.0 );\n"
					"gl_FragColor = originalColor;\n"
				"}\n";
	}

	void Ball::initializeVertexPositions() {
		float ballRadius = 0.1;
		int ballVertexCount = 16;
		float ballElementAngle = 3.14159265 / ballVertexCount;

		LOGI("ball angle = %f", ballElementAngle);


		elementPerVertex = 3;
		vertexPositionsSize = sizeof(float) * elementPerVertex * (ballVertexCount + 2); //+1 for center vertex, +1 for start and stop gets to be the same

		vertexPositionsPointer = (float*) malloc(vertexPositionsSize);
		vertexPositionsSize = vertexPositionsSize / sizeof(float);

		*vertexPositionsPointer = 0.0f;
		*(vertexPositionsPointer + 1) = 0.0f;
		*(vertexPositionsPointer + 2) = -1.15;
		for(int i=1;i <= ballVertexCount;i++){
			*(vertexPositionsPointer + (elementPerVertex * i)) = sin(	ballElementAngle * 2 * i) * ballRadius;
			*(vertexPositionsPointer + (elementPerVertex * i) + 1) = cos(	ballElementAngle * 2 * i) * ballRadius;
			*(vertexPositionsPointer + (elementPerVertex * i) + 2) = -1.15;
		}
		*(vertexPositionsPointer + (elementPerVertex * (ballVertexCount+1))) = sin(	ballElementAngle * 2 * (ballVertexCount+1)) * ballRadius;
		*(vertexPositionsPointer + (elementPerVertex * (ballVertexCount+1))+1) = cos(	ballElementAngle * 2 * (ballVertexCount+1)) * ballRadius;
		*(vertexPositionsPointer + (elementPerVertex * (ballVertexCount+1))+2) = -1.15;

	}

	GLuint Ball::CreateShader(GLenum eShaderType, const std::string &strShaderFile) {
		GLuint shader = glCreateShader(eShaderType);
		const char *strFileData = strShaderFile.c_str();
		glShaderSource(shader, 1, &strFileData, NULL);

		glCompileShader(shader);

		GLint status;

		glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
		if (status == GL_FALSE) {
			GLint infoLogLength;
			glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);

			GLchar *strInfoLog = new GLchar[infoLogLength + 1];
			glGetShaderInfoLog(shader, infoLogLength, NULL, strInfoLog);

			const char *strShaderType = NULL;
			switch (eShaderType) {
			case GL_VERTEX_SHADER:
				strShaderType = "vertex";
				break;
			case GL_FRAGMENT_SHADER:
				strShaderType = "fragment";
				break;
			}

			/*fprintf(stderr, "Compile failure in %s shader:\n%s\n",
					strShaderType, strInfoLog);
					*/
			LOGI("Compile failure in %s shader:\n%s\n", strShaderType, strInfoLog);
			delete[] strInfoLog;
		}

		return shader;
	}

	GLuint Ball::CreateProgram(const std::vector<GLuint> &shaderList) {
		GLuint program = glCreateProgram();

		for (size_t iLoop = 0; iLoop < shaderList.size(); iLoop++)
			glAttachShader(program, shaderList[iLoop]);
		glLinkProgram(program);

		GLint status;
		glGetProgramiv(program, GL_LINK_STATUS, &status);
		if (status == GL_FALSE) {
			GLint infoLogLength;
			glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLogLength);

			GLchar *strInfoLog = new GLchar[infoLogLength + 1];
			glGetProgramInfoLog(program, infoLogLength, NULL, strInfoLog);
			//fprintf(stderr, "Linker failure: %s\n", strInfoLog);
			LOGI("Linker failure: %s\n", strInfoLog);
			delete[] strInfoLog;
		}

		for (size_t iLoop = 0; iLoop < shaderList.size(); iLoop++)
			glDetachShader(program, shaderList[iLoop]);

		return program;
	}

	void Ball::initializeProgram() {
		std::vector<GLuint> shaderList;

		shaderList.push_back(CreateShader(GL_VERTEX_SHADER, this->VSbasic));
		shaderList.push_back(CreateShader(GL_FRAGMENT_SHADER, this->FSbasic));

		_ballGLSLProgram = CreateProgram(shaderList);

		std::for_each(shaderList.begin(), shaderList.end(), glDeleteShader);
	}

	void Ball::initializeVertexBuffer() {
		glGenBuffers(1, &positionBufferObject);

		glBindBuffer(GL_ARRAY_BUFFER, positionBufferObject);
		glBufferData(GL_ARRAY_BUFFER, vertexPositionsSize * sizeof(float),
				vertexPositionsPointer, GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	void Ball::initializePerspectiveMatrix(int height, int width){
		float fxFrustumScale = 1.0f;
		float fyFrustumScale = 1.0f;
		float fzNear = 0.5f;
		float fzFar = 3.0f;

		if(height > width){
			//x is bigger, so we will scale x.
			fxFrustumScale = (float)height / (float)width;
		} else {
			//y is bigger, scale y
			fyFrustumScale = (float)width / (float)height;

		}

		memset(perspectiveMatrix, 0, sizeof(perspectiveMatrix)); //set 0 to all elements.
		perspectiveMatrix[0] = fxFrustumScale;
		perspectiveMatrix[5] = fyFrustumScale;
		perspectiveMatrix[10] = (fzFar + fzNear) / (fzNear - fzFar);
		perspectiveMatrix[11] = -1.0f;
		perspectiveMatrix[14] = (2 * fzFar * fzNear) / (fzNear - fzFar);
		LOGI("init pers");
	}




	Ball::Ball(int height, int width) {
		fXOffset = 0.0f;
		fYOffset = 0.0f;

		initializeVertexShader();
		initializeFragmentShader();
		initializeProgram();
		initializeVertexPositions();
		initializeVertexBuffer();
		initializePerspectiveMatrix(height,width);

		positionBufferPointer = glGetAttribLocation(_ballGLSLProgram, "vPosition");
	}

	void Ball::draw(float xPosition, float yPosition) {

		glUseProgram(_ballGLSLProgram);

		GLint offsetLocation = glGetUniformLocation(_ballGLSLProgram, "offset");

		glUniform2f(offsetLocation, xPosition, yPosition);

		GLint perspectiveMatrixLocation = glGetUniformLocation(_ballGLSLProgram, "perspectiveMatrix");

		glUniformMatrix4fv(perspectiveMatrixLocation, 1, GL_FALSE, perspectiveMatrix);

		glBindBuffer(GL_ARRAY_BUFFER, positionBufferObject);
		glEnableVertexAttribArray(positionBufferPointer);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

		glDrawArrays(GL_TRIANGLE_FAN, 0,
				vertexPositionsSize / elementPerVertex);

		glDisableVertexAttribArray(positionBufferPointer);
		glUseProgram(0);
	}

}
