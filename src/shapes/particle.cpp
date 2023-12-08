#include "particle.h"

ParticleSystem::ParticleSystem()
{
    particles.clear();
    init_ParticleSystem();
}


void ParticleSystem::init_ParticleSystem()
{
    for(int i=0;i<maxparticles;i++){
        particles.push_back(init_particle());
        PosData.push_back(particles[i].position.x);
        PosData.push_back(particles[i].position.y);
        PosData.push_back(particles[i].position.z);
    }

}
particle ParticleSystem::init_particle(){
    std::uniform_real_distribution<> uniform_dis(0,1);
    std::uniform_real_distribution<> height_dis(1,3);
    std::uniform_real_distribution<> pi_dis(0,2*M_PI);

    // radomly generate x,z from 0-1
    float x=uniform_dis(m_gen);
    float z=uniform_dis(m_gen);

    //generate height pos from 3-7
    float y=height_dis(m_gen);
    float angle=pi_dis(m_gen);
    particle p;

    p.position=glm::vec3(x,y,z);
    p.velocity=glm::vec3(0,-0.6,0);
    p.theta=pi_dis(m_gen);
    p.phi=pi_dis(m_gen)/12;
    p.omega=(uniform_dis(m_gen)-0.5)/1;
    p.axis=glm::vec3(sin(p.phi)*sin(p.theta),cos(p.phi),sin(p.phi)*cos(p.theta));
    // generate accleration
    p.acceleration=glm::vec3(0.05*glm::cos(angle),0,0.05*glm::sin(angle));
    p.lifetime=std::min(3*y,5.0f);
    return p;

}
void ParticleSystem::update_ParticleSystem(float deltaTime){
    deltaT=deltaTime;
    QtConcurrent::map(particles, [this](particle &p) {
        update_particle(p);
    });

    update_particle_pos();
}


void ParticleSystem::update_particle(particle & p){
    if(!p.grounded&&p.lifetime>=0){
        std::uniform_real_distribution<> uniform_dis(0,1);
        std::uniform_real_distribution<> pi_dis(0,2*M_PI);
        // update particles data from fram to frame
        float angle=pi_dis(m_gen);
        p.position+=p.velocity*this->deltaT;
        p.velocity+=p.acceleration*this->deltaT;
        p.acceleration=glm::vec3(0.05*glm::cos(angle),0,0.05*glm::sin(angle));
        p.theta+=p.omega*deltaT;
        p.omega+=std::clamp((uniform_dis(m_gen)-0.5)*0.25,-3.0,3.0);
//        p.lifetime=p.lifetime-0.1*deltaT-0.01*p.position.y;
    }
    else {
        //kill grounded or lifetime<0 particle and re-generate them
        p=init_particle();
    }
}

void ParticleSystem::update_particle_pos(){
    PosData.clear();
    for(auto&particle:particles){
        PosData.push_back(particle.position.x);
        PosData.push_back(particle.position.y);
        PosData.push_back(particle.position.z);
    }
}
std::vector<float> ParticleSystem::getPosData(){
    return PosData;
}
