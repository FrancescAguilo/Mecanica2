#include <imgui\imgui.h>
#include <imgui\imgui_impl_sdl_gl3.h>
#include <glm\glm.hpp>
//#include <glm\vec3.hpp>
#include <iostream>
#include <glm/gtx/intersect.hpp>


//Exemple
extern void Exemple_GUI();
extern void Exemple_PhysicsInit();
extern void Exemple_PhysicsUpdate(float dt);
extern void Exemple_PhysicsCleanup();
extern bool renderSphere;

bool show_test_window = false;
void GUI() {
	bool show = true;
	ImGui::Begin("Physics Parameters", &show, 0);

	{
		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);//FrameRate
		//Exemple_GUI();
		ImGui::Checkbox("Esfera", &renderSphere);
	}

	ImGui::End();
}

namespace Sphere {
	glm::vec3 c = glm::vec3(0, 1, 0); //centro esfera
	float r = 1; //radio esfera 

	extern void setupSphere(glm::vec3 pos, float rad);
	extern void cleanupSphere();
	extern void updateSphere(glm::vec3 pos, float rad);
}

namespace ClothMesh {
	extern void setupClothMesh();
	extern void cleanupClothMesh();
	extern void updateClothMesh(float* array_data);
	extern void drawClothMesh();
}

namespace extraData {
	float indiceRebote = 2;

	//planos
	glm::vec3 XYn = glm::vec3(0, 0, 1);//normal de plano XY es Z
	glm::vec3 YZn = glm::vec3(1, 0, 0);
	glm::vec3 XZn = glm::vec3(0, 1, 0);
	glm::vec3 negYZn = glm::vec3(-1, 0, 0);
	glm::vec3 negXYn = glm::vec3(0, 0, -1);
	glm::vec3 negXZn = glm::vec3(0, -1, 0);

	glm::vec3 aux = glm::vec3(-5, 0, -5);
	glm::vec3 aux2 = glm::vec3(-5, 0, 5);
	glm::vec3 aux3 = glm::vec3(5, 0, -5);
	glm::vec3 aux4 = glm::vec3(-5, 10, -5);
	glm::vec3 vX1 = aux - aux2;
	glm::vec3 vX2 = aux - aux3;
	

	//esfera
	//glm::vec3 auxEsf;

	float planeD(glm::vec3 normal, glm::vec3 point) {
		return -(normal.x*point.x + normal.y*point.y + normal.z*point.z);
	}

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
		
		glm::vec3 velocity;
		//glm::vec3 lastVelocity;

		MeshPoint *rightPoint;
		MeshPoint *leftPoint;
		MeshPoint *upPoint;
		MeshPoint *downPoint;

		float LtoRight;
		float LtoLeft;
		float LtoUp;
		float LtoDown;

	};

	static struct ParticleMesh {
		MeshPoint **points;
		glm::vec3 *pointsPositions;
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

glm::vec3 getSpringForce(MeshPoint P1, MeshPoint P2 , float L12) {

	glm::vec3 auxVec3 =  (P1.actualPosition-P2.actualPosition) / glm::length<float>(P1.actualPosition - P2.actualPosition);
	float firstComponent = myPM.kEslatic * (glm::length<float>(P1.actualPosition - P2.actualPosition) - L12);
	float secondComponent = myPM.kDump * (glm::dot<float>(auxVec3,P1.velocity-P2.velocity));
	
	return -(firstComponent + secondComponent) * auxVec3;
}
////////////////////

void MyPhysicsInit() {
	//myPS.position = new glm::vec3[14*18];

	ClothMesh::setupClothMesh();

	myPM.points = new MeshPoint*[18];
	for (int i = 0; i < 18; i++) {
		myPM.points[i] = new MeshPoint[14];
	}
	myPM.pointsPositions = new glm::vec3[14*18];

	for (int i = 0; i < 18; i++) {
		for (int j = 0; j < 14; j++) {
			myPM.points[i][j].actualPosition.x = i * 0.5f - 17 * 0.5f * 0.5f; //posicio inicial = horitzontal i elevat
			myPM.points[i][j].actualPosition.y = 9;
			myPM.points[i][j].actualPosition.z = j * 0.5f - 13 * 0.5f * 0.5f;
			myPM.points[i][j].lastPosition = myPM.points[i][j].actualPosition;

			myPM.points[i][j].velocity = glm::vec3(0,0,0); //velocitat inicial = 0
			//myPM.points[i][j].lastVelocity = myPM.points[i][j].velocity;
			myPM.points[i][j].totalForce = glm::vec3(0, myPM.gravity, 0); //força total inicial = gravetat
			myPM.points[i][j].shearForce = glm::vec3(0, 0, 0);
			myPM.points[i][j].structuralForce = glm::vec3(0, 0, 0);
		    myPM.points[i][j].bendingForce = glm::vec3(0, 0, 0);



		}
	}
	updateMyPMPointsPositions();
	ClothMesh::updateClothMesh(&(myPM.pointsPositions[0].x));


}

void MyPhysicsUpdate(float dt) {
	//dt = dt * 0.025;
	//forçes
	//getSpringForce(glm::vec3(2, 2, 2), glm::vec3(2, 2, 1), 0);



	//actualitza posicions i velocitat apart amb Verlet
	for (int i = 0; i < 18; i++) {
		for (int j = 0; j < 14; j++) {

			myPM.points[i][j].newPosition = myPM.points[i][j].actualPosition + (myPM.points[i][j].actualPosition - myPM.points[i][j].lastPosition) + (myPM.points[i][j].totalForce / myPM.points[i][j].mass)*glm::pow(dt, 2);
			//std::cout << "ANTES DE COLISION" << myPM.points[i][j].newPosition.x << "," << myPM.points[i][j].newPosition.y << "," << myPM.points[i][j].newPosition.z << std::endl;
			


			//Colisions

			//plano tierra                                     positionI                                                                                                        positionF
			if (((glm::dot(extraData::XZn, myPM.points[i][j].lastPosition) + extraData::planeD(extraData::XZn, extraData::aux))*(glm::dot(extraData::XZn, myPM.points[i][j].actualPosition) + extraData::planeD(extraData::XZn, extraData::aux))) <= 0) {
				//std::cout << "COLISION" << std::endl;
				//std::cout << "ANTES:" << myPM.points[i][j].newPosition.x << "," << myPM.points[i][j].newPosition.y << "," << myPM.points[i][j].newPosition.z << std::endl;

				myPM.points[i][j].newPosition = myPM.points[i][j].actualPosition - 2 * (glm::dot(extraData::XZn , myPM.points[i][j].actualPosition) + extraData::planeD(extraData::XZn, extraData::aux))*extraData::XZn;
				
				//std::cout << "DESPUES DE COLISION:" << myPM.points[i][j].newPosition.x << "," << myPM.points[i][j].newPosition.y << "," << myPM.points[i][j].newPosition.z << "\n" <<std::endl;
				
			}

			//renderSphere = true;
			//Colisiones esfera
			if (renderSphere) {
				Sphere::updateSphere(Sphere::c, Sphere::r);
				glm::vec3 nPlane = (myPM.points[i][j].lastPosition - Sphere::c);

				if (glm::sqrt(glm::pow(nPlane.x, 2) + glm::pow(nPlane.y, 2) + glm::pow(nPlane.z, 2)) <= Sphere::r) {
					glm::vec3 punto;
					glm::vec3 normal;
					glm::intersectLineSphere(myPM.points[i][j].actualPosition, myPM.points[i][j].lastPosition, Sphere::c, Sphere::r, punto, normal);
					float D = -(normal.x*punto.x + normal.y*punto.y + normal.z*punto.z);

					myPM.points[i][j].newPosition = myPM.points[i][j].actualPosition - extraData::indiceRebote * (glm::dot(normal, myPM.points[i][j].actualPosition) + D)*normal;
					
				}

			}


			//Calcular forçes



			//Actualitzar posició
			myPM.points[i][j].lastPosition = myPM.points[i][j].actualPosition;

			myPM.points[i][j].velocity = (myPM.points[i][j].newPosition - myPM.points[i][j].actualPosition) / dt;

			myPM.points[i][j].actualPosition = myPM.points[i][j].newPosition;

		}
	}
	std::cout << "POSICION" << myPM.points[5][5].newPosition.x << "," << myPM.points[5][5].newPosition.y << "," << myPM.points[5][5].newPosition.z << "\n" << std::endl;

	updateMyPMPointsPositions();
	ClothMesh::updateClothMesh(&(myPM.pointsPositions[0].x));
	
}

void MyPhysicsCleanup() {
	ClothMesh::cleanupClothMesh();
	delete[] myPM.points;
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