#include "lab_m1/Tema2/Tema2.h"

#include <vector>
#include <string>
#include <iostream>

using namespace std;
using namespace m1;

const int n = 20;
const int m = 20;

/*
 *  To find out more about `FrameStart`, `Update`, `FrameEnd`
 *  and the order in which they are called, see `world.cpp`.
 */


Tema2::Tema2()
{
}


Tema2::~Tema2()
{
}


void Tema2::Init()
{
    renderCameraTarget = false;
    
    //for the perspective projection
    fov = 60;
    ZNear = 0.01f;
    ZFar = 200.0f;

    bodyAngle = 0;

    playerX = 0; 
    playerY = 0.3; 
    playerZ = 0;

    camera = new implemented::Tema2_camera();
    firstCamera = false;
    secondCamera = false;

     // Create a shader program for drawing face polygon with the color of the normal
    {
        Shader *shader = new Shader("MyShader");
        shader->AddShader(PATH_JOIN(window->props.selfDir, SOURCE_PATH::M1, "Tema2", "shaders", "VertexShader.glsl"), GL_VERTEX_SHADER);
        shader->AddShader(PATH_JOIN(window->props.selfDir, SOURCE_PATH::M1, "Tema2", "shaders", "FragmentShader.glsl"), GL_FRAGMENT_SHADER);
        shader->CreateAndLink();
        shaders[shader->GetName()] = shader;
    }

    {
        Mesh* mesh = new Mesh("box");
        mesh->LoadMesh(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::MODELS, "primitives"), "box.obj");
        meshes[mesh->GetMeshID()] = mesh;
    }

    {
        Mesh* mesh = new Mesh("sphere");
        mesh->LoadMesh(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::MODELS, "primitives"), "sphere.obj");
        meshes[mesh->GetMeshID()] = mesh;
    }

    {
        Mesh* mesh = new Mesh("plane");
        mesh->LoadMesh(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::MODELS, "primitives"), "plane50.obj");
        meshes[mesh->GetMeshID()] = mesh;
    }

    projectionMatrix = glm::perspective(RADIANS(fov), window->props.aspectRatio, ZNear, ZFar);
    perspective = true;


    GenerateMaze();
    camera->Set(glm::vec3(playerX, playerY + 1, playerZ + 2), glm::vec3(playerX, playerY, playerZ), glm::vec3(0, 1, 0));
    camera->distanceToTarget = 2;
    thirdPerson = true;
}

void Tema2::FrameStart()

{
    // Clears the color buffer (using the previously set color) and depth buffer
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::ivec2 resolution = window->GetResolution();
    // Sets the screen area where to draw
    glViewport(0, 0, resolution.x, resolution.y);
}

void Tema2::GenerateMaze()
{
    /*Choose a random cell.
   Choose a random neighbor of the current cell and visit it. If the neighbor has not yet been visited, add the traveled edge to the spanning tree.
   Repeat step 2 until all cells have been visited.
   0 - drum liber (fara cub)
   1 - va reprezenta un perete din labirint (vom plasa un cub)
   2 - un inamic*/
    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < m; j++)
        {
            maze[i][j] = 1;
        }
    }

    srand(time(0));

    int start_x = rand() % (n - 3) + 1;
    int start_y = rand() % (n - 3) + 1;
    int totalcells = (n ) * (m) / 4;

    int nr_enemies = totalcells / 10;
    if (totalcells % 2 == 0) totalcells++;

    if (start_x % 2 == 1) start_x++;
    if (start_y % 2 == 1) start_y++;

    playerX = (float) start_x;
    playerZ = (float) start_y;

    maze[start_x][start_y] = 0;
    totalcells--;

    while (totalcells) {
        int direction = rand() % 4;
        // 1 - east
        // 2 - west
        // 3 - south

        if (direction == 0)
        {
            //we try to go north ^ if it is not visited
            if (start_y >= 3)
            {
                start_y -= 2;
                if (maze[start_x][start_y] == 1) {
                    maze[start_x][start_y] = 0;
                    maze[start_x][start_y + 1] = 0;

                    totalcells -= 2;
                }
            }
        }
        else if (direction == 1)
            //we try to go east >
        {
            if ((start_x + 2) <= n - 2)
            {
                start_x += 2;
                if (maze[start_x][start_y] == 1) {
                    maze[start_x][start_y] = 0;
                    maze[start_x - 1][start_y] = 0;

                    totalcells -= 2;
                }
            }
        }
        else if (direction == 2)
            //we try to go west <
        {
            if (start_x >= 3)
            {
                start_x -= 2;
                if (maze[start_x][start_y] == 1)
                {
                    maze[start_x][start_y] = 0;
                    maze[start_x + 1][start_y] = 0;

                    totalcells -= 2;
                }
            }
        }
        else if (direction == 3)
        {
            //we try to go south if it is not visited
            if ((start_y + 2) <= m - 2)
            {
                start_y += 2;
                if (maze[start_x][start_y] == 1)
                {
                    maze[start_x][start_y] = 0;
                    maze[start_x][start_y - 1] = 0;

                    totalcells -= 2;
                }

            }
        }
    }

    for (int i = 2; i < m; i++)
    {
        if (maze[1][i] == 0)
        {
            maze[0][i] = 0;
            break;
        }
        if (maze[n - 2][i] == 0)
        {
            maze[n - 1][i] = 0;
            break;
        }
    }

    for (int i = 2; i < n; i++)
    {
        if (maze[i][1] == 0)
        {
            maze[i][0] = 0;
            break;
        }
        if (maze[i][n - 2] == 0)
        {
            maze[i][n - 1] = 0;
            break;
        }
    }

    //spawning enemies
    for (int i = 2; i < n; i+=2)
    {
        for (int j = 2; j < m; j+=2)
        {
            if (maze[i][j] == 0 && i != playerX && j != playerZ)
            {
                int inamic = rand() % 2;
                if (inamic == 1 && nr_enemies !=0)
                {
                    maze[i][j] = 2;
                    nr_enemies--;
                    break;
                }
            }
        }
    }
}

void Tema2::Update(float deltaTimeSeconds)
{
    playerX = camera->GetTargetPosition().x;
    playerY = camera->GetTargetPosition().y;
    playerZ = camera->GetTargetPosition().y;
    if(thirdPerson)
        RenderPlayer(camera->GetTargetPosition());
    RenderMaze();
}

void Tema2::RenderPlayer(glm::vec3 position) {
    //position of the body
    float x = position.x;
    float y = position.y;
    float z = position.z;

    float dx = camera->position.x - x;
    float dy = camera->position.z - z;

    bodyAngle = M_PI_2 -atan2(dy, dx);

    //torso
    glm::mat4 modelMatrix = glm::mat4(1);
    /*modelMatrix = glm::translate(modelMatrix, glm::vec3(x, y, z));
    modelMatrix = glm::rotate(modelMatrix, bodyAngle, glm::vec3(0, 1, 0));
    modelMatrix = glm::translate(modelMatrix, glm::vec3(-x, -y, -z));*/
    modelMatrix = glm::translate(modelMatrix, position);
    modelMatrix = glm::rotate(modelMatrix, bodyAngle, glm::vec3(0, 1, 0));
    modelMatrix = glm::scale(modelMatrix, glm::vec3(0.25f, 0.30f, 0.15f));
    
    RenderSimpleMesh(meshes["box"], shaders["MyShader"], modelMatrix, glm::vec3(1, 0, 0.5));
    
    //capul
    modelMatrix = glm::mat4(1);
    modelMatrix = glm::translate(modelMatrix, glm::vec3(x, y, z));
    modelMatrix = glm::rotate(modelMatrix, bodyAngle, glm::vec3(0, 1, 0));
    modelMatrix = glm::translate(modelMatrix, glm::vec3(-x, -y, -z));
    modelMatrix = glm::translate(modelMatrix, glm::vec3(x, y + 0.22f, z));
    modelMatrix = glm::scale(modelMatrix, glm::vec3(0.12f, 0.12f, 0.12f));
    RenderSimpleMesh(meshes["box"], shaders["MyShader"], modelMatrix, glm::vec3(0.5, 0, 0.5));

    //mana dreapta
    modelMatrix = glm::mat4(1); 
    modelMatrix = glm::translate(modelMatrix, glm::vec3(x, y, z));
    modelMatrix = glm::rotate(modelMatrix, bodyAngle, glm::vec3(0, 1, 0));
    modelMatrix = glm::translate(modelMatrix, glm::vec3(-x, -y, -z));
    modelMatrix = glm::translate(modelMatrix, glm::vec3(x+ .19f, y +0.01, z));
    modelMatrix = glm::scale(modelMatrix, glm::vec3(0.09f, 0.25f, 0.09f));
    RenderSimpleMesh(meshes["box"], shaders["MyShader"], modelMatrix, glm::vec3(0.5, 0, 0.5));
    
    //mana stanga
    modelMatrix = glm::mat4(1);
    modelMatrix = glm::translate(modelMatrix, glm::vec3(x, y, z));
    modelMatrix = glm::rotate(modelMatrix, bodyAngle, glm::vec3(0, 1, 0));
    modelMatrix = glm::translate(modelMatrix, glm::vec3(-x, -y, -z));
    modelMatrix = glm::translate(modelMatrix, glm::vec3(x - .19f, y + 0.01, z));
    modelMatrix = glm::scale(modelMatrix, glm::vec3(0.09f, 0.25f, 0.09f));
    RenderSimpleMesh(meshes["box"], shaders["MyShader"], modelMatrix, glm::vec3(0.5, 0, 0.5));

    //piciorul drept
    modelMatrix = glm::mat4(1);
    modelMatrix = glm::translate(modelMatrix, glm::vec3(x, y, z));
    modelMatrix = glm::rotate(modelMatrix, bodyAngle, glm::vec3(0, 1, 0));
    modelMatrix = glm::translate(modelMatrix, glm::vec3(-x, -y, -z));
    modelMatrix = glm::translate(modelMatrix, glm::vec3(x + .09f, y - 0.28f, z));
    modelMatrix = glm::scale(modelMatrix, glm::vec3(0.09f, 0.22f, 0.09f));
    RenderSimpleMesh(meshes["box"], shaders["MyShader"], modelMatrix, glm::vec3(0.5, 0, 1));

    //piciorul stang
    modelMatrix = glm::mat4(1);
    modelMatrix = glm::translate(modelMatrix, glm::vec3(x, y, z));
    modelMatrix = glm::rotate(modelMatrix, bodyAngle, glm::vec3(0, 1, 0));
    modelMatrix = glm::translate(modelMatrix, glm::vec3(-x, -y, -z));
    modelMatrix = glm::translate(modelMatrix, glm::vec3(x- .09f, y - .28f, z));
    modelMatrix = glm::scale(modelMatrix, glm::vec3(0.09f, 0.22f, 0.09f));
    RenderSimpleMesh(meshes["box"], shaders["MyShader"], modelMatrix, glm::vec3(0.5, 0, 1));
}

void Tema2::RenderMaze()
{
    glm::mat4 modelMatrix = glm::mat4(1);
    modelMatrix = glm::translate(modelMatrix, glm::vec3(10, 0.01f, 10));
    modelMatrix = glm::scale(modelMatrix, glm::vec3(0.5f));
    RenderSimpleMesh(meshes["plane"], shaders["MyShader"], modelMatrix, glm::vec3(0, 0.2, 0.2));
    
    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < m; j++) {
            if (maze[i][j] == 1)
            {
                modelMatrix = glm::mat4(1);
                modelMatrix = glm::translate(modelMatrix, glm::vec3(i, 0.28, j));
                modelMatrix = glm::scale(modelMatrix, glm::vec3(1, 0.5, 1));
                RenderSimpleMesh(meshes["box"], shaders["MyShader"], modelMatrix, glm::vec3(0.2,0,0.2));
            }
            else
                if (maze[i][j] == 2)
                {
                    RenderEnemy(i, j);
                }
        }
    }
}

void Tema2::RenderEnemy(int i, int j)
{
    GLfloat time = Engine::GetElapsedTime();
    glm::mat4 modelMatrix = glm::mat4(1);
    float sinus = sin(time*5) * 0.2;
    float cosinus = cos(time*5) *0.2;
    modelMatrix = glm::translate(modelMatrix, glm::vec3( i + cosinus, 0.4, j - sinus));
    modelMatrix = glm::scale(modelMatrix, glm::vec3(0.6, 0.8, 0.6));
    RenderSimpleMesh(meshes["sphere"], shaders["MyShader"], modelMatrix, glm::vec3(1, 0, 0));
}


void Tema2::FrameEnd()
{
}


void Tema2::RenderMesh(Mesh * mesh, Shader * shader, const glm::mat4 & modelMatrix)
{
    if (!mesh || !shader || !shader->program)
        return;

    // Render an object using the specified shader and the specified position
    shader->Use();
    glUniformMatrix4fv(shader->loc_view_matrix, 1, GL_FALSE, glm::value_ptr(camera->GetViewMatrix()));
    glUniformMatrix4fv(shader->loc_projection_matrix, 1, GL_FALSE, glm::value_ptr(projectionMatrix));
    glUniformMatrix4fv(shader->loc_model_matrix, 1, GL_FALSE, glm::value_ptr(modelMatrix));

    mesh->Render();
}

void Tema2::RenderSimpleMesh(Mesh* mesh, Shader* shader, const glm::mat4& modelMatrix, const glm::vec3& color)
{
    if (!mesh || !shader || !shader->GetProgramID())
        return;

    // Render an object using the specified shader and the specified position
    glUseProgram(shader->program);

    glUniformMatrix4fv(shader->loc_model_matrix, 1, GL_FALSE, glm::value_ptr(modelMatrix));
    glUniformMatrix4fv(shader->loc_view_matrix, 1, GL_FALSE, glm::value_ptr(camera->GetViewMatrix()));
    glUniformMatrix4fv(shader->loc_projection_matrix, 1, GL_FALSE, glm::value_ptr(projectionMatrix));

    GLint loc_object = glGetUniformLocation(shader->program, "color");
    glUniform3fv(loc_object, 1, glm::value_ptr(color));

    // Draw the object
    glBindVertexArray(mesh->GetBuffers()->m_VAO);
    glDrawElements(mesh->GetDrawMode(), static_cast<int>(mesh->indices.size()), GL_UNSIGNED_INT, 0);
}

/*
 *  These are callback functions. To find more about callbacks and
 *  how they behave, see `input_controller.h`.
 */


void Tema2::OnInputUpdate(float deltaTime, int mods)
{
    // move the camera only if MOUSE_RIGHT button is pressed
    if (window->MouseHold(GLFW_MOUSE_BUTTON_RIGHT))
    {
        float cameraSpeed = 2.0f;

        if (window->KeyHold(GLFW_KEY_W)) {
            //Translate the camera forward
            camera->MoveForward(deltaTime * cameraSpeed);
        }

        if (window->KeyHold(GLFW_KEY_A)) {
            //Translate the camera to the left
            camera->TranslateRight(-deltaTime * cameraSpeed);
        }

        if (window->KeyHold(GLFW_KEY_S)) {
            //Translate the camera backward
            camera->MoveForward(-deltaTime * cameraSpeed);
        }

        if (window->KeyHold(GLFW_KEY_D)) {
            //Translate the camera to the right
            camera->TranslateRight(deltaTime * cameraSpeed);
        }
    }
    
}


void Tema2::OnKeyPress(int key, int mods)
{
    //change first person or third person camera when you press control
    if (key == GLFW_KEY_LEFT_CONTROL || key == GLFW_KEY_RIGHT_CONTROL)
    {
        if (thirdPerson)
        {
            //first person
            thirdPerson = false;
            camera->TranslateForward(camera->distanceToTarget);
        }
        else
        {
            thirdPerson = true;
            camera->TranslateForward(-camera->distanceToTarget);
        }
    }
}


void Tema2::OnKeyRelease(int key, int mods)
{
    // Add key release event
}


void Tema2::OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY)
{
    // Add mouse move event

    //

    if (window->MouseHold(GLFW_MOUSE_BUTTON_RIGHT))
    {
        float sensivityOX = 0.001f;
        float sensivityOY = 0.001f;

        if (!thirdPerson) {
            camera->RotateFirstPerson_OX(sensivityOX * -deltaY); //vertical
            angle = sensivityOY * -deltaX;
            camera->RotateFirstPerson_OY(sensivityOY * -deltaX);
        }

        if (thirdPerson) {
            camera->RotateThirdPerson_OX(sensivityOX * -deltaY);
            angle = sensivityOY * -deltaX;
            camera->RotateThirdPerson_OY(sensivityOY * -deltaX);
        }
    }
}


void Tema2::OnMouseBtnPress(int mouseX, int mouseY, int button, int mods)
{
    // Add mouse button press event
}


void Tema2::OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods)
{
    // Add mouse button release event
}


void Tema2::OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY)
{
}


void Tema2::OnWindowResize(int width, int height)
{
}
