#include <iostream>
#include <iomanip>
#include <sstream>
#include <filesystem>

#include "manta.h"
#include "grid.h"
#include "levelset.h"
#include "particle.h"
#include "mesh.h"
#include "shapes.h"
#include "flip.h"
#include "fastmarch.h"
#include "extforces.h"
#include "pressure.h"

namespace fs = std::experimental::filesystem;

int main()
{
    // get grid resolution:
    int res(0);
    do
    {
        std::cout << "Enter grid resolution (between 10 and 500): ";
        std::cin >> res;
    } while ((res < 10) || (res > 500));

    // check for existing output paths, create if necessary
    if (!fs::exists("simulation data")) fs::create_directory("simulation data");
    if (!fs::exists("simulation data/DropObjMesh")) fs::create_directory("simulation data/DropObjMesh");

    int  dimension  = 3;
    auto resolution = Manta::Vec3i(res);
    if (dimension == 2) resolution.z = 1;

    auto main_solver = Manta::FluidSolver(resolution, dimension);
    main_solver.setTimeStep(0.5f);
    Real minParticles = pow(2, dimension);

    // size of particles
    Real radiusFactor = 1.0f;

    // solver grids
    auto flags      = Manta::FlagGrid(&main_solver);
    auto phi        = Manta::LevelsetGrid(&main_solver);
    auto vel        = Manta::MACGrid(&main_solver);
    auto vel_old    = Manta::MACGrid(&main_solver);
    auto pressure   = Manta::Grid<Real>(&main_solver);
    auto tmp_vec3   = Manta::Grid<Manta::Vec3>(&main_solver);

    // particles and mesh
    auto pp         = Manta::BasicParticleSystem(&main_solver);
    auto pVel       = Manta::PdataVec3(&pp);
    auto mesh       = Manta::Mesh(&main_solver);

    // acceleration data for particle nbs
    auto pindex     = Manta::ParticleIndexSystem(&main_solver);
    auto gpi        = Manta::Grid<int>(&main_solver);

    int setup = 0;
    int boundaryWidth = 1;
    flags.initDomain(boundaryWidth);

    // drop mesh into box
    auto fluidbasin = Manta::Box(&main_solver, Manta::Vec3::Invalid,
                                               Manta::Vec3(0.0f, 0.0f, 0.0f),
                                               Manta::Vec3(resolution.x * 1.0f, resolution.y * 0.1f, resolution.z * 1.0f));

    phi.copyFrom(fluidbasin.computeLevelset());

    // import obj
    auto objMesh = Manta::Mesh(&main_solver);
    objMesh.load("..\\..\\manta\\tools\\tests\\Suzanne.obj");
    objMesh.scale(Manta::Vec3(resolution.x / 3.0f));
    objMesh.offset(resolution.x * 0.5f);

    // rasterize obj
    auto objGrid = Manta::LevelsetGrid(&main_solver);
    objMesh.computeLevelset(objGrid, 2.0f);
    phi.join(objGrid);

    flags.updateFromLevelset(phi);
    Manta::sampleLevelsetWithParticles(phi, flags, pp, 2, 0.05f);

    Manta::mapGridToPartsVec3(vel, pp, pVel);

    for (int t = 0; t < 250; t++)
    {
        // FLIP
        pp.advectInGrid(flags, vel, Manta::IntegrationMode::IntRK4, false);

        // make sure we have velocities throughout liquid region
        Manta::mapPartsToMAC(flags, vel, vel_old, pp, pVel, &tmp_vec3);
        Manta::extrapolateMACFromWeight(vel, tmp_vec3, 2);
        Manta::markFluidCells(pp, flags);

        // create approximate surface level set, resample particles
        Manta::gridParticleIndex(pp, pindex, flags, gpi);
        Manta::unionParticleLevelset(pp, pindex, flags, gpi, phi, radiusFactor);
        Manta::extrapolateLsSimple(phi, 4, true);

        // forces and pressure solve
        Manta::addGravity(flags, vel, Manta::Vec3(0.0f, -0.003f, 0.0f));
        Manta::setWallBcs(flags, vel);
        Manta::solvePressure(vel, pressure, flags, 1e-3, &phi);
        Manta::setWallBcs(flags, vel);

        // set source grids for resampling, used in adjustNumber!
        pVel.setSource(&vel, true);
        Manta::adjustNumber(pp, vel, flags, 1 * minParticles, 2 * minParticles, phi, radiusFactor);

        // make sure we have proper velocities
        Manta::extrapolateMACSimple(flags, vel);
        Manta::flipVelocityUpdate(flags, vel, vel_old, pp, pVel, 0.97f);

        if (dimension == 3) phi.createMesh(mesh);

        main_solver.step();

        // generate blender mesh and particle data for DropObjMesh_resurfaced surface generation scene
        std::stringstream filename1, filename2;
        filename1 << "simulation data\\DropObjMesh\\fluidsurface_final_" << std::setw(4) << std::setfill('0') << t << ".bobj.gz";
        filename2 << "simulation data\\DropObjMesh\\flipParts_"          << std::setw(4) << std::setfill('0') << t << ".uni";
        mesh.save(filename1.str());
        pp.save(filename2.str());
    }
}
