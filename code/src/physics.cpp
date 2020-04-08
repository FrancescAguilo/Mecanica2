#include <imgui\imgui.h>
#include <imgui\imgui_impl_sdl_gl3.h>
#include <glm\glm.hpp>


//Exemple
extern void Exemple_GUI();
extern void Exemple_PhysicsInit();
extern void Exemple_PhysicsUpdate(float dt);
extern void Exemple_PhysicsCleanup();


bool show_test_window = false;
void GUI() {
	bool show = true;
	ImGui::Begin("Physics Parameters", &show, 0);

	{
		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);//FrameRate
		Exemple_GUI();
	}

	ImGui::End();
}

namespace ClothMesh {
	extern void setupClothMesh();
	extern void cleanupClothMesh();
	extern void updateClothMesh(float* array_data);
	extern void drawClothMesh();
}

namespace {
	static struct PhysParams {
		float min = 0.f;
		float max = 10.f;
	} p_pars;

	static struct ParticleSystem {
		glm::vec3 *position;
		int numParticles;
	} myPS;

	static struct MeshPoint {
		glm::vec3 position;
		glm::vec3 totalForce; //total
		glm::vec3 structuralForce; //horitzontals i verticals
		glm::vec3 shearForce; //diagonals
		glm::vec3 bendingForce; //contaria?
		float mass;

		MeshPoint *rightPoint;
		MeshPoint *leftPoint;
		MeshPoint *upPoint;
		MeshPoint *downPoint;
	};

	static struct ParticleMesh {

		MeshPoint **previusPoints;//punts anteriors
		MeshPoint **actualPoints;//punts actuals
		MeshPoint **nextPoints;//punts següents

		glm::vec3 *pointsPositions;//?
		float kEslatic = 1;
		float kDump = 1;

	} myPM;
}

////////////////////

void MyPhysicsInit() {
	//myPS.position = new glm::vec3[14*18];

	ClothMesh::setupClothMesh();

	myPM.actualPoints = new MeshPoint*[18];
	for (int i = 0; i < 18; i++) {
		myPM.actualPoints[i] = new MeshPoint[14];
	}
	myPM.pointsPositions = new glm::vec3[14*18];

	for (int i = 0; i < 18; i++) {
		for (int j = 0; j < 14; j++) {
			myPM.actualPoints[i][j].position.x = i * 0.5f - 17 * 0.5f * 0.5f;
			myPM.actualPoints[i][j].position.y = 9;
			myPM.actualPoints[i][j].position.z = j * 0.5f - 13 * 0.5f * 0.5f;
			myPM.pointsPositions[i*14 + j] = glm::vec3(myPM.actualPoints[i][j].position.x, myPM.actualPoints[i][j].position.y, myPM.actualPoints[i][j].position.z);
			
		}
	}
	ClothMesh::updateClothMesh(&(myPM.pointsPositions[0].x));

	//ClothMesh::updateClothMesh(&(myPM.points[0][0].position.x));
	//ClothMesh::updateClothMesh(&(myPS.position[0].x));

	//for (int i = 0; i < 18; i++) {
	//	for (int j = 0; j < 14; j++) {
	//		myPM.points[i][j].position.x = myPM.points[i][j].position.x + j * 0.5;
	//		myPM.points[i][j].position.y = 5;
	//		myPM.points[i][j].position.z = myPM.points[i][j].position.z + i * 0.5;

	//		//myPS.position[i + j] = glm::vec3(i, 5, j);
	//	}

	//}
	//ClothMesh::updateClothMesh(&(myPS.position[0].x));
	//ClothMesh::updateClothMesh(&(myPM.points[0][0].position.x));
}

void MyPhysicsUpdate(float dt) {
	
	//verlet
	for (int i = 0; i < 18; i++) {
		for (int j = 0; j < 14; j++) {
			//caluclar força total


			//canviar posicions
			//posicionSiguiente=posicionActual + (posicionActual - posicionAnterior) + (f/m)* dt^2
			//FALTA dt QUE NO SE QUE ES
			//myPM.nextPoints[i][j].position = myPM.actualPoints[i][j].position + (myPM.actualPoints[i][j].position - myPM.previusPoints[i][j].position) + (myPM.actualPoints[i][j].totalForce / myPM.actualPoints[i][j].mass)/* * dt^2 */;

		}
	}




	
	//ClothMesh::updateClothMesh(&(myPM.points[0][0].position.x));
}

void MyPhysicsCleanup() {
	ClothMesh::cleanupClothMesh();
	delete[] myPM.actualPoints;
}






/////////////////////////////////

void PhysicsInit() {
	MyPhysicsInit();
}

void PhysicsUpdate(float dt) {
	MyPhysicsUpdate(dt);
}

void PhysicsCleanup() {
	MyPhysicsCleanup();
}