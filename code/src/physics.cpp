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
		MeshPoint *points;
		float kEslatic = 1;
		float kDump = 1;

	} myPM;
}

////////////////////

void MyPhysicsInit() {
	ClothMesh::setupClothMesh();
}

void MyPhysicsUpdate(float dt) {
	//ClothMesh::updateClothMesh();
	ClothMesh::drawClothMesh();
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