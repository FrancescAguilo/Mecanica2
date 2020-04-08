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

	/*static struct ParticleSystem {
		glm::vec3 *position;
		int numParticles;
	} myPS;*/

	static struct MeshPoint {
		glm::vec3 newPosition;
		glm::vec3 actualPosition;
		glm::vec3 lastPosition;
		glm::vec3 totalForce; //total
		glm::vec3 structuralForce; //horitzontals i verticals
		glm::vec3 shearForce; //diagonals
		glm::vec3 bendingForce; //contaria?
		float mass = 1;
		
		/*glm::vec3 velocity;
		glm::vec3 lastVelocity;*/

		MeshPoint *rightPoint;
		MeshPoint *leftPoint;
		MeshPoint *upPoint;
		MeshPoint *downPoint;
	};

	static struct ParticleMesh {

		MeshPoint **previusPoints;//punts anteriors
		MeshPoint **actualPoints;//punts actuals
		MeshPoint **nextPoints;//punts seg�ents

		glm::vec3 *pointsPositions;//?
		float kEslatic = 1;
		float kDump = 1;
		float gravity = -9.81;

	} myPM;
}
void updateMyPMPointsPositions() {
	for (int i = 0; i < 18; i++) {
		for (int j = 0; j < 14; j++) {
			myPM.pointsPositions[i * 14 + j] = glm::vec3(myPM.points[i][j].actualPosition.x, myPM.points[i][j].actualPosition.y, myPM.points[i][j].actualPosition.z);
		}
	}
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
			myPM.points[i][j].actualPosition.x = i * 0.5f - 17 * 0.5f * 0.5f; //posicio inicial = horitzontal i elevat
			myPM.points[i][j].actualPosition.y = 9;
			myPM.points[i][j].actualPosition.z = j * 0.5f - 13 * 0.5f * 0.5f;
			myPM.points[i][j].lastPosition = myPM.points[i][j].actualPosition;

			//myPM.points[i][j].velocity = glm::vec3(0,0,0); //velocitat inicial = 0
			//myPM.points[i][j].lastVelocity = myPM.points[i][j].velocity;
			myPM.points[i][j].totalForce = glm::vec3(0, myPM.gravity, 0); //for�a total inicial = gravetat
			myPM.points[i][j].shearForce = glm::vec3(0, 0, 0);
			myPM.points[i][j].structuralForce = glm::vec3(0, 0, 0);
		    myPM.points[i][j].bendingForce = glm::vec3(0, 0, 0);
		}
	}
	updateMyPMPointsPositions();
	ClothMesh::updateClothMesh(&(myPM.pointsPositions[0].x));


}

void MyPhysicsUpdate(float dt) {
	
	//actualitza posicions
	for (int i = 0; i < 18; i++) {
		for (int j = 0; j < 14; j++) {

			
			myPM.points[i][j].newPosition = myPM.points[i][j].actualPosition + (myPM.points[i][j].actualPosition - myPM.points[i][j].lastPosition) + (myPM.points[i][j].totalForce / myPM.points[i][j].mass)*glm::pow(dt, 2);
			myPM.points[i][j].lastPosition = myPM.points[i][j].actualPosition;
			myPM.points[i][j].actualPosition = myPM.points[i][j].newPosition;
		}
	}


	updateMyPMPointsPositions();
	ClothMesh::updateClothMesh(&(myPM.pointsPositions[0].x));
	
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