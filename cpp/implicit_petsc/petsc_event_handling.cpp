#include "petsc_event_handling.h"
#include <chrono>
#include <thread>
#include "utils_ch_implicit.h"
#include <stdio.h>
#include <iostream>
#include <fstream>

PetscErrorCode EventFunction( TS ts , PetscReal t , Vec U , PetscScalar *fvalue , void *ctx ) {

  AppCtx            *app=(AppCtx*)ctx;
  PetscErrorCode    ierr;

  // Event 1: stop for driver program 
  if ( (t - (app->dt_counter + 1) * app->dt_check) < 0 )
    fvalue[0] = 1;
  else
    fvalue[0] = 0;

  // Event 2: output solution
  if ( (t - (app->dt_output_counter + 1) * app->dt_output) < 0 )
    fvalue[1] = 1;
  else
    fvalue[1] = 0;


  return(0);

}

PetscErrorCode PostEventFunction(TS ts,PetscInt nevents,PetscInt event_list[],PetscReal t,Vec U,PetscBool forwardsolve,void* ctx) {

  AppCtx         *app=(AppCtx*)ctx;
  PetscReal       m, m_new;
  
  if ( (event_list[0] == 0) && ( t < app->t_final ) ) {

    const std::string name     = "m_" + std::to_string( (app->dt_counter + 1) * app->dt_check ).substr(0,4) + ".dat";
    const std::string petscout = "Attepting to read new m at t = %5.2f seconds from file " + name + "\n";
    PetscPrintf( PETSC_COMM_WORLD , petscout.c_str() , (double)t );

    // Output to file that you are ready for a new value
    const std::string outname  = "complete_" + std::to_string( (app->dt_counter + 1) * app->dt_check ).substr(0,4) + ".dat";
    std::ofstream fout(outname);
    fout << "complete\n";
    fout.close();

    // Wait until receive a new value of m from an input file
    while(true) {

      if (FILE *file = fopen(name.c_str(), "r") ) {

        fclose(file);
        
        std::this_thread::sleep_for( std::chrono::milliseconds(100) ); // Give the driver time to finish writing the new parameters

        std::ifstream fin(name);
        PetscReal m_new;
        fin >> m_new;
        
        app->m           = m_new;
        app->dt_counter += 1;
        
        PetscPrintf( PETSC_COMM_WORLD , "Changing m at t = %5.2f seconds to m = %5.2f\n" , (double)t , (double)app->m );

        fin.close();        
        break;
        
      }

    }
  }

  if ( (event_list[1] == 1) && ( t < app->t_final ) ) {

    PetscPrintf( PETSC_COMM_WORLD , "Logging solution at t = %5.2f seconds\n" , (double)t );

    const std::string outname = "c_" + std::to_string( (app->dt_output_counter + 1) * app->dt_output ).substr(0,4) + ".dat";

    const PetscScalar *u;

    PetscViewer    viewer;
    PetscViewerCreate( PETSC_COMM_WORLD , &viewer );
    PetscViewerSetType( viewer , PETSCVIEWERASCII );
    PetscViewerFileSetMode( viewer , FILE_MODE_WRITE );

    PetscViewerASCIIOpen( PETSC_COMM_WORLD , outname.c_str() , &viewer );
    VecView( U , viewer );
    
    PetscViewerDestroy( &viewer );
    
    app->dt_output_counter += 1;

  }

  return(0);

}
