// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <iostream>
#include <ctime>
#include <chrono>
#include <map>
#include <chrono>
#include <thread>


// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <GLFW/glfw3.h>
GLFWwindow* window;

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include <common/shader.hpp>
#include <common/texture.hpp>
#include <common/controls.hpp>
#include <common/objloader.hpp>



static int uniID = 0;

int generateId() {
    return uniID++;
}

struct movingPoint {
    movingPoint(){}

    movingPoint(float a, float b, float c, glm::vec3 d){
        this->x = a;
        this->y = b;
        this->z = c;
        this->direction = d;
    }
    float x;
    float y;
    float z;
    glm::vec3 direction;
};

using pointsSet = std::map<int, movingPoint>;

// some global vectors. Did not won't to send them as arguments

// vertices for 1 monster
std::vector<glm::vec3> monster_vertices;
std::vector<glm::vec2> monster_uvs;
std::vector<glm::vec3> monster_normals;

// vertices for 1 fireball
std::vector<glm::vec3> ball_vertices;
std::vector<glm::vec2> ball_uvs;
std::vector<glm::vec3> ball_normals;

const float object_speed = 4.0f;
const float monstr_speed = 5.0f;

// all vertices in current window
std::vector<glm::vec3> vertices;
std::vector<glm::vec2> uvs;
std::vector<glm::vec3> normals;


void moveObjects(pointsSet& points, float time){
    try {

        glm::vec3 my_pos =  getPosition();
        float my_x = my_pos.x;
        float my_y = my_pos.y;
        float my_z = my_pos.z;

        for(auto it = points.begin(); it != points.end(); ) {
            if ((it->second.x - my_x) * (it->second.x - my_x) +
                (it->second.y - my_y) * (it->second.y - my_y) +
                (it->second.z - my_z) * (it->second.z - my_z) > 1000) {
                it = points.erase(it);
            }
            else {
                it->second.x += it->second.direction.x * object_speed * time;
                it->second.y += it->second.direction.y * object_speed * time;
                it->second.z += it->second.direction.z * object_speed * time;
                ++it;
            }
        }
    }
    catch (const char* msg) {
        printf(msg);

    }
}

std::vector<glm::vec3> makeVertexVector(const pointsSet& centres, const std::vector<vec3>& model_vertices) {
    std::vector<glm::vec3> result;
    for(auto &[key, point]: centres) {
        result.insert(result.end(), model_vertices.begin(), model_vertices.end());
        for(size_t i = result.size() - model_vertices.size(); i < result.size(); ++i) {
            result[i].x += point.x;
            result[i].y += point.y;
            result[i].z += point.z;
        }
    }
    return result;
}

void showAllObjects(pointsSet& monster_centres, pointsSet& balls_centres, GLuint& vertexbuffer, GLuint& uvbuffer) {
    vertices.clear();
    uvs.clear();
    auto all_monster_vertices = makeVertexVector(monster_centres, monster_vertices);
    auto all_balls_vertives = makeVertexVector(balls_centres, ball_vertices);
    vertices.insert(vertices.end(), all_monster_vertices.begin(), all_monster_vertices.end());
    vertices.insert(vertices.end(), all_balls_vertives.begin(), all_balls_vertives.end());
    for(int j = 0; j < monster_centres.size(); j++){
        uvs.insert(uvs.end(), monster_uvs.begin(), monster_uvs.end());
    }
    for(int j = 0; j < balls_centres.size(); j++){
        uvs.insert(uvs.end(), ball_uvs.begin(), ball_uvs.end());
    }
    glGenBuffers(1, &vertexbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);

    glGenBuffers(1, &uvbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
    glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec2), &uvs[0], GL_STATIC_DRAW);
}

void CreateNewMonster(pointsSet& monster_centers){
    // can be generated in one place or near gamer
    float theta =  0 + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(3.142f-0)));
    float phi =  0 + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(6.283f-0)));
    float rad =  1 + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(50.0f-15)));
    float x_ = rad*cos(phi)*sin(theta);
    float y_ = rad*sin(phi)*sin(theta);
    float z_ = rad*cos(theta);

    monster_centers.emplace(generateId(), movingPoint(x_, y_, z_, glm::vec3(0, 0, 0)));
}

void fire(pointsSet& balls_centers){
    // can be generated in one place or near gamer
    auto current_position = getPosition();
    balls_centers.emplace(generateId(), movingPoint(current_position.x, current_position.y, current_position.z, getDirection()));
}

void removeCollisions(pointsSet& balls, pointsSet& monsters) {
    std::cout << balls.size() << std::endl;
    for(auto ball_it = balls.begin(); ball_it != balls.end(); ) {
        for(auto monster_it = monsters.begin(); monster_it != monsters.end(); ) {
            if ((ball_it->second.x - monster_it->second.x) * (ball_it->second.x - monster_it->second.x) +
                (ball_it->second.y - monster_it->second.y) * (ball_it->second.y - monster_it->second.y) +
                (ball_it->second.z - monster_it->second.z) * (ball_it->second.z - monster_it->second.z) < 1) {
                monsters.erase(monster_it);
                monster_it = monsters.begin();
                ball_it = balls.erase(ball_it);
            } else {
                ++monster_it;
            }
        }
        if (ball_it != balls.end()) {
            ++ball_it;
        }
    }
}

int main( void )
{
	// Initialise GLFW
	if( !glfwInit() )
	{
		fprintf( stderr, "Failed to initialize GLFW\n" );
		getchar();
		return -1;
	}

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Open a window and create its OpenGL context
	window = glfwCreateWindow( 1024, 768, "Game", NULL, NULL);
	if( window == NULL ){
		fprintf( stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n" );
		getchar();
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	// Initialize GLEW
	glewExperimental = true; // Needed for core profile
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		getchar();
		glfwTerminate();
		return -1;
	}

	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
    // Hide the mouse and enable unlimited mouvement
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    
    // Set the mouse at the center of the screen
    glfwPollEvents();
    glfwSetCursorPos(window, 1024/2, 768/2);

	// Dark blue background
	glClearColor(184.0f/255.0f, 230.0f/255.0f, 244.0f/255.0f, 0.0f);

	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS); 

	// Cull triangles which normal is not towards the camera
	glEnable(GL_CULL_FACE);

	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	// Create and compile our GLSL program from the shaders
	GLuint programID = LoadShaders( "TransformVertexShader.vertexshader", "TextureFragmentShader.fragmentshader" );

	// Get a handle for our "MVP" uniform
	GLuint MatrixID = glGetUniformLocation(programID, "MVP");

	// Load the texture
	//GLuint Texture = loadDDS("uvmap.DDS");
	//GLuint Texture = loadDDS("ice512.dds");
	//GLuint Texture = loadDDS("lava.dds");
	GLuint Texture = loadDDS("ice512.dds");
	//GLuint Texture = loadDDS("orange_skin.dds");

	// Get a handle for our "myTextureSampler" uniform
	GLuint TextureID  = glGetUniformLocation(programID, "myTextureSampler");

	// Read our .obj file
	//bool res = loadOBJ("cube.obj", vertices, uvs, normals);
	bool monster_res = loadOBJ("myobj.obj", monster_vertices, monster_uvs, monster_normals);
	bool ball_res = loadOBJ("sphere.obj", ball_vertices, ball_uvs, ball_normals);
	//bool res = loadOBJ("Orange.obj", monster_vertices, monster_uvs, monster_normals);
	//bool res = loadOBJ("MONARCH.OBJ", vertices, uvs, normals);
    //bool res = loadOBJ("Butterfly.obj", vertices, uvs, normals);
    //bool res = loadOBJ("LADYBUG.OBJ", vertices, uvs, normals);


    // ball should be smaller, so we recalculate vertices
    for(int k = 0; k < ball_vertices.size(); k++) {
        ball_vertices[k].x /= 40;
        ball_vertices[k].y /= 40;
        ball_vertices[k].z /= 40;
    }

    // All monsters
    pointsSet monster_centers;
    monster_centers.emplace(generateId(), movingPoint(0, 0, 0, glm::vec3(0,0,0)));

    // All balls
    std::vector<glm::vec3> ball_directions;
    pointsSet ball_centers;



    GLuint vertexbuffer;
    GLuint uvbuffer;

    auto ball_last_time = float(glfwGetTime());
    auto monstr_last_time = float(glfwGetTime());

    // Load it into a VBO

	glGenBuffers(1, &vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);

	glGenBuffers(1, &uvbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
	glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec2), &uvs[0], GL_STATIC_DRAW);

	//int iteration_number = 0;
    srand(26);



    auto last_monster_generation = std::chrono::system_clock::now();
    auto last_fireball_generation = std::chrono::system_clock::now();
    auto cur_time = std::chrono::system_clock::now();


    std::cout<<"Starting game"<<std::endl;

	do{
	    // get my current position in space
        glm::vec3 my_position = getPosition();

        cur_time = std::chrono::system_clock::now();
        std::chrono::duration<double> elapsed_seconds = cur_time - last_monster_generation;

        // create monsters every 2 sec
	    if(elapsed_seconds.count() > 2 && monster_centers.size() < 20) {
             CreateNewMonster(monster_centers);
            // should we check for intersection with old monsters? /TODO

            //update time
            last_monster_generation = std::chrono::system_clock::now();
	    }
	    // fire fireball, if ENTER is pressed
        if (glfwGetKey( window, GLFW_KEY_ENTER ) == GLFW_PRESS &&
            ball_centers.size() < 100 &&
            (cur_time - last_fireball_generation).count() > 100000000) {

            last_fireball_generation = std::chrono::system_clock::now();
            fire(ball_centers);
            auto ball_delta_time = float(glfwGetTime() - ball_last_time);
        }
        // balls fly each in their direction


        showAllObjects(monster_centers, ball_centers, vertexbuffer, uvbuffer);
        // moving balls
        auto ball_delta_time = float(glfwGetTime() - ball_last_time);
        moveObjects(ball_centers, ball_delta_time);
        ball_last_time = float(glfwGetTime());

        removeCollisions(ball_centers, monster_centers);
        //if(number_of_monsters > 0){
        //    moveMonsters();
        //}



		// Clear the screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Use our shader
		glUseProgram(programID);

		// Compute the MVP matrix from keyboard and mouse input
		computeMatricesFromInputs();
		glm::mat4 ProjectionMatrix = getProjectionMatrix();
		glm::mat4 ViewMatrix = getViewMatrix();
		glm::mat4 ModelMatrix = glm::mat4(1.0);
		glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;

		// Send our transformation to the currently bound shader, 
		// in the "MVP" uniform
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);

		// Bind our texture in Texture Unit 0
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, Texture);
		// Set our "myTextureSampler" sampler to use Texture Unit 0
		glUniform1i(TextureID, 0);

		// 1rst attribute buffer : vertices
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		glVertexAttribPointer(
			0,                  // attribute
			3,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
		);

		// 2nd attribute buffer : UVs
		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
		glVertexAttribPointer(
			1,                                // attribute
			2,                                // size
			GL_FLOAT,                         // type
			GL_FALSE,                         // normalized?
			0,                                // stride
			(void*)0                          // array buffer offset
		);

		// Draw the triangle !
		glDrawArrays(GL_TRIANGLES, 0, vertices.size() );

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);

		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();
	} // Check if the ESC key was pressed or the window was closed
	while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
		   glfwWindowShouldClose(window) == 0 );




	// Cleanup VBO and shader
	glDeleteBuffers(1, &vertexbuffer);
	glDeleteBuffers(1, &uvbuffer);
	glDeleteProgram(programID);
	glDeleteTextures(1, &Texture);
	glDeleteVertexArrays(1, &VertexArrayID);

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;
}

