#include <limits>
#include <sstream>
#include <algorithm>

#ifdef _OPENMP
#include <omp.h>
#endif

#include <PhotonParticleContainer.H>
#include <WarpX_f.H>
#include <WarpX.H>
#include <WarpXConst.H>


// Import low-level single-particle kernels
#include <UpdatePositionPhoton.H>


using namespace amrex;

PhotonParticleContainer::PhotonParticleContainer (AmrCore* amr_core, int ispecies,
                                                  const std::string& name)
    : PhysicalParticleContainer(amr_core, ispecies, name)
{}

void PhotonParticleContainer::InitData()
{
    AddParticles(0); // Note - add on level 0

    if (maxLevel() > 0) {
        Redistribute();  // We then redistribute
    }
}

void
PhotonParticleContainer::PushPX(WarpXParIter& pti,
                                Cuda::ManagedDeviceVector<ParticleReal>& xp,
                                Cuda::ManagedDeviceVector<ParticleReal>& yp,
                                Cuda::ManagedDeviceVector<ParticleReal>& zp,
                                Real dt, DtType a_dt_type)
{

    // This wraps the momentum and position advance so that inheritors can modify the call.
    auto& attribs = pti.GetAttribs();
    // Extract pointers to the different particle quantities
    ParticleReal* const AMREX_RESTRICT x = xp.dataPtr();
    ParticleReal* const AMREX_RESTRICT y = yp.dataPtr();
    ParticleReal* const AMREX_RESTRICT z = zp.dataPtr();
    ParticleReal* const AMREX_RESTRICT ux = attribs[PIdx::ux].dataPtr();
    ParticleReal* const AMREX_RESTRICT uy = attribs[PIdx::uy].dataPtr();
    ParticleReal* const AMREX_RESTRICT uz = attribs[PIdx::uz].dataPtr();
    const ParticleReal* const AMREX_RESTRICT Ex = attribs[PIdx::Ex].dataPtr();
    const ParticleReal* const AMREX_RESTRICT Ey = attribs[PIdx::Ey].dataPtr();
    const ParticleReal* const AMREX_RESTRICT Ez = attribs[PIdx::Ez].dataPtr();
    const ParticleReal* const AMREX_RESTRICT Bx = attribs[PIdx::Bx].dataPtr();
    const ParticleReal* const AMREX_RESTRICT By = attribs[PIdx::By].dataPtr();
    const ParticleReal* const AMREX_RESTRICT Bz = attribs[PIdx::Bz].dataPtr();

    if (WarpX::do_boosted_frame_diagnostic && do_boosted_frame_diags)
    {
        copy_attribs(pti, x, y, z);
    }

    //No need to update momentum for photons (for now)

    amrex::ParallelFor(
        pti.numParticles(),
        [=] AMREX_GPU_DEVICE (long i) {

            UpdatePositionPhoton( x[i], y[i], z[i],
                            ux[i], uy[i], uz[i], dt );
        }
    );

}

void
PhotonParticleContainer::Evolve (int lev,
                                 const MultiFab& Ex, const MultiFab& Ey, const MultiFab& Ez,
                                 const MultiFab& Bx, const MultiFab& By, const MultiFab& Bz,
                                 MultiFab& jx, MultiFab& jy, MultiFab& jz,
                                 MultiFab* cjx, MultiFab* cjy, MultiFab* cjz,
                                 MultiFab* rho, MultiFab* crho,
                                 const MultiFab* cEx, const MultiFab* cEy, const MultiFab* cEz,
                                 const MultiFab* cBx, const MultiFab* cBy, const MultiFab* cBz,
                                 Real t, Real dt, DtType a_dt_type)
{
    // This does gather, push and depose.
    // Push and depose have been re-written for photon,
    // so they do not do anything.
    PhysicalParticleContainer::Evolve (lev,
                                       Ex, Ey, Ez,
                                       Bx, By, Bz,
                                       jx, jy, jz,
                                       cjx, cjy, cjz,
                                       rho, crho,
                                       cEx, cEy, cEz,
                                       cBx, cBy, cBz,
                                       t, dt);

}
