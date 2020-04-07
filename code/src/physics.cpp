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
		MeshPoint **points;
		float kEslatic = 1;
		float kDump = 1;

	} myPM;
}

////////////////////

void MyPhysicsInit() {
	ClothMesh::setupClothMesh();

	myPM.points = new MeshPoint*[18];
	for (int i = 0; i < 14; i++) {
		myPM.points[i] = new MeshPoint[14];
	}

	for (int i = 0; i < 18; i++) {
		for (int j = 0; j < 14; j++) {
			myPM.points[i][j].position.x = -3;
			myPM.points[i][j].position.y = 9;
			myPM.points[i][j].position.z = 0;
		}
	}
	ClothMesh::updateClothMesh(&(myPM.points[0][0].position.x));

	for (int i = 0; i < 18; i++) {
		for (int j = 0; j < 14; j++) {
			myPM.points[i][j].position.x = myPM.points[i][j].position.x + j * 0.5;
			myPM.points[i][j].position.y = 5;
			myPM.points[i][j].position.z = myPM.points[i][j].position.z + i * 0.5;
		}

	}

	ClothMesh::updateClothMesh(&(myPM.points[0][0].position.x));
}

void MyPhysicsUpdate(float dt) {
	



	//ClothMesh::updateClothMesh(&(myPM.points[0][0].position.x));
	
}

void MyPhysicsCleanup() {
	ClothMesh::cleanupClothMesh();
	delete[] myPM.points;
}






/////////////////////////////////

void PhysicsInit() {

}

void PhysicsUpdate(float dt) {

}

void PhysicsCleanup() {

}