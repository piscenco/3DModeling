// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <iostream>
#include <ctime>
#include <chrono>

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



struct point {
    point(){}

    point(float a, float b, float c){
        this->x = a;
        this->y = b;
        this->z = c;
    }
    float x;
    float y;
    float z;
};

struct ballCenter {
    ballCenter(){}

    ballCenter(float a, float b, float c, glm::vec3 d){
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

// some global vectors. Did not won't to send them as arguments

// vertices for 1 monster
std::vector<glm::vec3> monster_vertices;
std::vector<glm::vec2> monster_uvs;
std::vector<glm::vec3> monster_normals;

int number_of_monsters = 0;
std::vector<glm::vec3> all_monsters_vertices;
std::vector<point> monster_centers;

// vertices for 1 fireball
std::vector<glm::vec3> ball_vertices;
std::vector<glm::vec2> ball_uvs;
std::vector<glm::vec3> ball_normals;

int number_of_balls = 0;
std::vector<glm::vec3> all_balls_vertices;
std::vector<glm::vec3> ball_directions;
std::vector<point> ball_centers;

// all vertices in current window
std::vector<glm::vec3> vertices;
std::vector<glm::vec2> uvs;
std::vector<glm::vec3> normals;

float ball_speed = 6.0f;
float monstr_speed = 5.0f;

GLuint vertexbuffer;
GLuint uvbuffer;

float ball_last_time = float(glfwGetTime());
float monstr_last_time = float(glfwGetTime());



void moveMonsters(){
    try {
        glm::vec3 my_pos =  getPosition();
        float my_x = my_pos.x;
        float my_y = my_pos.y;
        float my_z = my_pos.z;
        float monstr_current_time = glfwGetTime();
        auto monstr_delta_time = float(monstr_current_time - monstr_last_time);
        int v_in_monstr = monster_vertices.size();
        float m_x;
        float m_y;
        float m_z;
        for (int i_monstr = 0; i_monstr < all_monsters_vertices.size(); i_monstr++) {

            m_x = (my_x - all_monsters_vertices[i_monstr].x);
            m_y = (my_y - all_monsters_vertices[i_monstr].y);
            m_z = (my_z - all_monsters_vertices[i_monstr].z);
            glm::vec3 move_vec = glm::normalize(glm::vec3(m_x, m_y, m_z)) * 0.5f;// * monstr_speed * monstr_delta_time;
            all_monsters_vertices[i_monstr ] -= move_vec;

        }

        for (int i_monstr = 0; i_monstr < number_of_monsters; i_monstr++) {
            m_x = my_x - all_monsters_vertices[i_monstr].x;
            m_y = my_y - all_monsters_vertices[i_monstr].y;
            m_z = my_z - all_monsters_vertices[i_monstr].z;
            glm::vec3 move_vec = glm::normalize(glm::vec3(m_x, m_y, m_z))* 0.5f;// monstr_speed * monstr_delta_time;
            monster_centers[i_monstr ].x -= move_vec.x;
            monster_centers[i_monstr ].y -= move_vec.y;
            monster_centers[i_monstr ].z -= move_vec.z;
        }
        monstr_last_time = float(glfwGetTime());
    }
    catch (const char* msg) {
        printf(msg);
    }
}

void moveBalls(){
    try {
        // only 1 ball
        // ball that is father than 10 disappears
        glm::vec3 my_pos =  getPosition();
        float my_x = my_pos.x;
        float my_y = my_pos.y;
        float my_z = my_pos.z;
       if (ball_centers[0].x - my_x > 10 || ball_centers[0].y - my_y > 10 || ball_centers[0].z - my_z> 10 ||
                ball_centers[0].x - my_x < -10 || ball_centers[0].y - my_y < -10 || ball_centers[0].z - my_z< -10) {
            number_of_balls = 0;
            all_balls_vertices.clear();
            ball_directions.clear();
            ball_centers.clear();
            number_of_balls = 0;
        }
        float ball_current_time = glfwGetTime();
        auto ball_delta_time = float(ball_current_time - ball_last_time);
        int v_in_ball = ball_vertices.size();
        float m_x;
        float m_y;
        float m_z;
        for (int i_ball = 0; i_ball < number_of_balls; i_ball++) {
            for (int v_ball = 0; v_ball < v_in_ball; v_ball++) {
                m_x = ball_directions[i_ball].x * ball_speed * ball_delta_time;
                m_y = ball_directions[i_ball].y * ball_speed * ball_delta_time;
                m_z = ball_directions[i_ball].z * ball_speed * ball_delta_time;
                glm::vec3 move_vec = glm::vec3(m_x, m_y, m_z);
                all_balls_vertices[v_in_ball * i_ball + v_ball] += move_vec;
            }
            ball_centers[i_ball].x += m_x;
            ball_centers[i_ball].y += m_y;
            ball_centers[i_ball].z += m_z;
        }
        ball_last_time = float(glfwGetTime());
    }
    catch (const char* msg) {
        printf(msg);
        
    }
}

void showAllObjects() {
    vertices.clear();
    uvs.clear();
    vertices = all_monsters_vertices;
    vertices.insert(vertices.end(), all_balls_vertices.begin(), all_balls_vertices.end());
    for(int j = 0; j < number_of_monsters; j++){
        uvs.insert(uvs.end(), monster_uvs.begin(), monster_uvs.end());
    }
    for(int j = 0; j < number_of_balls; j++){
        uvs.insert(uvs.end(), ball_uvs.begin(), ball_uvs.end());
    }
    glGenBuffers(1, &vertexbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);

    glGenBuffers(1, &uvbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
    glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec2), &uvs[0], GL_STATIC_DRAW);
}

void CreateNewMonster(){
    // can be generated in one place or near gamer
    std::vector<glm::vec3> new_monster_vertices = monster_vertices;
    std::vector<glm::vec2> new_monster_uvs = monster_uvs;
    std::vector<glm::vec3> new_monster_normals = monster_normals;

    int random_variable = std::rand();
    float theta =  0 + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(3.142f-0)));
    float phi =  0 + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(6.283f-0)));
    float rad =  1 + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(50.0f-15)));
    float x_ = rad*cos(phi)*sin(theta);
    float y_ = rad*sin(phi)*sin(theta);
    float z_ = rad*cos(theta);

    for(int j = 0; j < new_monster_vertices.size(); j++){
        new_monster_vertices[j].x+= x_;
        new_monster_vertices[j].y += y_;
        new_monster_vertices[j].z += z_;
        all_monsters_vertices.push_back(new_monster_vertices[j]);

    }
    number_of_monsters++;
    point p = point(x_, y_, z_);
    monster_centers.push_back(p);
}

void fire(){
    // can be generated in one place or near gamer
    std::vector<glm::vec3> new_ball_vertices = ball_vertices;

    float x_ = getPosition().x;
    float y_ = getPosition().y;
    float z_ = getPosition().z;


    for(int j = 0; j < new_ball_vertices.size(); j++){
        new_ball_vertices[j].x+= x_;
        new_ball_vertices[j].y += y_;
        new_ball_vertices[j].z += z_;
        all_balls_vertices.push_back(new_ball_vertices[j]);

    }
    ball_directions.push_back(getDirection());
    number_of_balls++;
    point p = point(x_, y_, z_);
    ball_centers.push_back(p);
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


    // Load it into a VBO

	glGenBuffers(1, &vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);

	glGenBuffers(1, &uvbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
	glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec2), &uvs[0], GL_STATIC_DRAW);
	//int iteration_number = 0;
    srand(26);

    // centers of generated monsters
    std::vector<point> centers;
    centers.emplace_back(0, 0, 0);

    auto last_monster_generation = std::chrono::system_clock::now();
    auto cur_time = std::chrono::system_clock::now();

    //point new_center = CreateNewMonster();

    std::cout<<"Starting game"<<std::endl;

	do{
	    // get my current position in space
        glm::vec3 my_position = getPosition();

        cur_time = std::chrono::system_clock::now();
        std::chrono::duration<double> elapsed_seconds = cur_time - last_monster_generation;

        // create monsters every 2 sec
	    if(elapsed_seconds.count() > 2) {
             CreateNewMonster();
            // should we check for intersection with old monsters?

            //update time
            last_monster_generation = std::chrono::system_clock::now();
	    }
	    // fire fireball, if ENTER is pressed
        if ((glfwGetKey( window, GLFW_KEY_ENTER ) == GLFW_PRESS )&& number_of_balls == 0){
            fire();
        }
        // balls fly each in their direction


        showAllObjects();

        if(number_of_balls > 0) {
            moveBalls();
        }
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

