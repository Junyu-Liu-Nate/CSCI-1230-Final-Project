#ifndef PARTICLE_H
#define PARTICLE_H
#include "sphere.h"
#include "cube.h"
#include <memory>
#include <random>
#include <vector>
#include <glm/gtc/matrix_transform.hpp>
#include <QtConcurrent/QtConcurrent>
#include <algorithm>
struct particle
{
    glm::vec3 position;
    glm::vec3 velocity;
    glm::vec3 acceleration;
    glm::vec3 axis;
    float theta;
    float phi;
    float omega;
    float lifetime;
    bool grounded=false;

};

class ParticleSystem
{
public:
    ParticleSystem();
    ParticleSystem(int num){
        maxparticles=num;
        init_ParticleSystem();
    };
    int getNum(){return maxparticles;}
    void updateNum(int new_num){
        if(new_num!=maxparticles){
            maxparticles=new_num;
            std::cout<<maxparticles<<std::endl;
            init_ParticleSystem();
            update_ParticleSystem(0.0);

        }
    }
    void update_ParticleSystem(float deltaT);
    std::vector<float> getPosData();
    int getParticleNum(){return maxparticles;}
    std::vector<glm::mat4> getModel(){
        std::vector<glm::mat4> particleModel;
        for(auto & particle:particles){
            glm::mat4 model=glm::translate(glm::mat4(1.0f), particle.position)*
                         glm::scale(glm::mat4(1.0f), glm::vec3(0.007f, 0.007f, 0.007f))*
                              glm::rotate(glm::mat4(1.0f),particle.theta,particle.axis);
            particleModel.push_back(model);
        }
        return particleModel;
    }

    std::vector<particle>& getParticles() {
        return particles;
    }
private:
    void init_ParticleSystem();

    particle init_particle();

    void update_particle(particle & p);
    void update_particle_pos();

    std::vector<particle>particles;
    std::vector<float>PosData;
    int maxparticles=1000;

    float deltaT=0.001;
    std::random_device rd;
    std::mt19937 m_gen=std::mt19937(rd());

};

#endif // PARTICLE_H
