/*---------------------------------------------------------------------------*\
     ██╗████████╗██╗  ██╗ █████╗  ██████╗ █████╗       ███████╗██╗   ██╗
     ██║╚══██╔══╝██║  ██║██╔══██╗██╔════╝██╔══██╗      ██╔════╝██║   ██║
     ██║   ██║   ███████║███████║██║     ███████║█████╗█████╗  ██║   ██║
     ██║   ██║   ██╔══██║██╔══██║██║     ██╔══██║╚════╝██╔══╝  ╚██╗ ██╔╝
     ██║   ██║   ██║  ██║██║  ██║╚██████╗██║  ██║      ██║      ╚████╔╝
     ╚═╝   ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═╝ ╚═════╝╚═╝  ╚═╝      ╚═╝       ╚═══╝

 * In real Time Highly Advanced Computational Applications for Finite Volumes
 * Copyright (C) 2017 by the ITHACA-FV authors
-------------------------------------------------------------------------------
License
    This file is part of ITHACA-FV
    ITHACA-FV is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
    ITHACA-FV is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU Lesser General Public License for more details.
    You should have received a copy of the GNU Lesser General Public License
    along with ITHACA-FV. If not, see <http://www.gnu.org/licenses/>.
Class
    ITHACAPOD
Description
    Implementation of a POD decomposition of a general field
SourceFiles
    ITHACAPOD.C
\*---------------------------------------------------------------------------*/

/// \file
/// Header file of the ITHACAPOD class.
/// \dir
/// Directory containing the header and source files for the ITHACAPOD class.

#ifndef ITHACAPOD_H
#define ITHACAPOD_H

#include "fvCFD.H"
#include "ITHACAutilities.H"
#include "ITHACAstream.H"
#include "ITHACAparameters.H"
#include "Foam2Eigen.H"
#include "EigenFunctions.H"
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic ignored "-Wnon-virtual-dtor"
#include <Spectra/GenEigsSolver.h>
#include <Spectra/SymEigsSolver.h>
#include <Eigen/Eigen>
#include <unsupported/Eigen/SparseExtra>
#pragma GCC diagnostic pop

/*---------------------------------------------------------------------------*\
                        Class reductionProblem Declaration
\*---------------------------------------------------------------------------*/

/// Class of the computation of the POD, it exploits the method of snapshots.
class ITHACAPOD
{

    public:

        // Public Members

        //--------------------------------------------------------------------------
        /// Compute the bases or read them for a vector field
        ///
        /// @param[in]  snapshots   a PtrList of volVectorField or volScalarField snapshots.
        /// @param[out] modes       a PtrList of volVectorField or volScalarField snapshots where modes are stored (it must be passed empty).
        /// @param[in]  podex       boolean variable 1 if the modes have been already computed and stored (in this case the function is reading them) 0 elsewhere.
        /// @param[in]  supex       boolean variable 1 if the supremizer modes have been already computed and stored (in this case the function is reading them) 0 elsewhere.
        /// @param[in]  sup         boolean variable 1 if you want to compute the supremizer modes 0 elsewhere.
        /// @param[in]  nmodes      int variable to set the number of modes to be stored, if set to 0 the maximum number of modes will computed.
        ///
        /// @tparam     Field_type  It can be vector or scalar
        ///
        template<class Field_type>
        static void getModes(PtrList<GeometricField<Field_type, fvPatchField, volMesh>>&
                             snapshots, PtrList<GeometricField<Field_type, fvPatchField, volMesh>>& modes,
                             bool podex, bool supex = 0, bool sup = 0, int nmodes = 0);

        //--------------------------------------------------------------------------
        /// @brief      Gets the bases for a scalar field using SVD instead of the method of snapshots
        ///
        /// @param[in]  snapshots   a PtrList of volVectorField or volScalarField snapshots.
        /// @param[out] modes       a PtrList of volVectorField or volScalarField snapshots where modes are stored (it must be passed empty).
        /// @param[in]  podex       boolean variable 1 if the modes have been already computed and stored (in this case the function is reading them) 0 elsewhere.
        /// @param[in]  supex       boolean variable 1 if the supremizer modes have been already computed and stored (in this case the function is reading them) 0 elsewhere.
        /// @param[in]  sup         boolean variable 1 if you want to compute the supremizer modes 0 elsewhere.
        /// @param[in]  nmodes      int variable to set the number of modes to be stored, if set to 0 the maximum number of modes will computed.
        ///
        /// @tparam     Field_type  It can be vector or scalar
        ///
        template<class Field_type>
        static void getModesSVD(
            PtrList<GeometricField<Field_type, fvPatchField, volMesh>>&
            snapshots, PtrList<GeometricField<Field_type, fvPatchField, volMesh>>& modes,
            bool podex, bool supex = 0, bool sup = 0, int nmodes = 0);


        //--------------------------------------------------------------------------
        /// Nested-POD approach. Compute the nested snapshot matrix and weighted bases for a vector field
        ///
        /// @param[in]  snapshots       a PtrList of volVectorField or volScalarField snapshots.
        /// @param[out] UModesGlobal    a PtrList of volVectorField or volScalarField snapshots where modes are stored (it must be passed empty).
        /// @param[in]  Npar            int variable which is equal with the number of parameters
        /// @param[in]  NnestedOut      int variable to set the number of weighted modes to be stored for the assembly of the nested matrix
        ///
        /// @tparam     Field_type  It can be vector or scalar
        ///
        template<class Field_type>
        static void getNestedSnapshotMatrix(
            PtrList<GeometricField<Field_type, fvPatchField, volMesh>>& snapshots,
            PtrList<GeometricField<Field_type, fvPatchField, volMesh>>& modes, int Npar,
            int NnestedOut);


        //--------------------------------------------------------------------------

        /// Compute the weighted bases (using the nested-pod approach) or read them for a vector field
        ///
        /// @param[in]  snapshots   a PtrList of volVectorField or volScalarField snapshots.
        /// @param[out] modes       a PtrList of volVectorField or volScalarField snapshots where modes are stored (it must be passed empty).
        /// @param[in]  podex       boolean variable 1 if the modes have been already computed and stored (in this case the function is reading them) 0 elsewhere.
        /// @param[in]  supex       boolean variable 1 if the supremizer modes have been already computed and stored (in this case the function is reading them) 0 elsewhere.
        /// @param[in]  sup         boolean variable 1 if you want to compute the supremizer modes 0 elsewhere.
        /// @param[in]  nmodes      int variable to set the number of modes to be stored, if set to 0 the maximum number of modes will computed.
        ///
        /// @tparam     Field_type  It can be vector or scalar
        ///
        template<class Field_type>
        static void getWeightedModes(
            PtrList<GeometricField<Field_type, fvPatchField, volMesh>>& snapshots,
            PtrList<GeometricField<Field_type, fvPatchField, volMesh>>& modes, bool podex,
            bool supex = 0, bool sup = 0,
            int nmodes = 0);

        //--------------------------------------------------------------------------
        /// @brief      Perform GrammSchmidt orthonormalization on an Eigen Matrix
        ///
        /// @param      Matrix  The matrix
        ///
        static void GrammSchmidt(Eigen::MatrixXd& Matrix);

        //--------------------------------------------------------------------------
        /// Computes the correlation matrix given a vector field snapshot Matrix using different norms depending on the input snapshots
        ///
        /// @param[in] snapshots    a list of snapshots in different format
        ///
        /// @return    the Eigen::MatrixXd correlation matrix.
        ///
        /// @tparam     Field_type  Type of the input list you are passing, it can be: PtrList<volVectorField>, PtrList<volScalarField>, List<Eigen::SparseMatrix<double>>, List<Eigen::VectorXd>.
        ///
        template<class Field_type>
        static Eigen::MatrixXd corMatrix(Field_type& snapshots);


        //--------------------------------------------------------------------------
        /// Export the basis for an OpenFOAM GeometricField into the ITHACAOutput/POD or ITHACAOutput/supremizer
        ///
        /// @param[in] s        a PtrList of GeometricField where the snapshots associated with the bases are stored.
        /// @param[in] bases    a PtrList of GeometricField where the bases are stored.
        /// @param[in] sup      a boolean variable 1 if you want to export the supremizer bases (in ITHACAOutput/supremizer) 0 elsewhere (Default is 0).
        ///
        /// @tparam     Field_type  It can be vector or scalar
        ///
        template<class Field_type>
        static void exportBases(
            PtrList<GeometricField<Field_type, fvPatchField, volMesh>>& snapshots,
            PtrList<GeometricField<Field_type, fvPatchField, volMesh>>& bases,
            bool sup = 0);

        //--------------------------------------------------------------------------
        /// Normalize the Bases for a GeometricField of OpenFOAM
        ///
        /// @param[in] s    a PtrList of GeometricFields where the bases are stored.
        ///
        /// @tparam     Field_type  It can be vector or scalar
        ///
        template<class Field_type>
        static void normalizeBases(
            PtrList<GeometricField<Field_type, fvPatchField, volMesh>>& bases);

        //--------------------------------------------------------------------------
        /// Normalize a volScalarField field using a volVectorField
        ///
        /// @param[in] Bases   a PtrList of volVectorField where the bases used for normalization are stored.
        /// @param[in] p       a PtrList of volScalarField  where the bases are stored.
        ///
        static void normalizeBases(PtrList<volVectorField>& Bases,
                                   PtrList<volScalarField>& p);

        //--------------------------------------------------------------------------
        /// Export the eigenvalues as a txt file
        ///
        /// @param[in] Eigenvalues  a scalarField of eigenvalues.
        /// @param[in] name         a fileName to indicate the name of the txt file.
        /// @param[in] sup          a boolean variable 1 if you want to export the eigenValues (in ITHACAOutput/supremizer) 0 elsewhere (Default is 0).
        ///
        static void exportEigenvalues(scalarField Eigenvalues, fileName name,
                                      bool sup = 0);

        //--------------------------------------------------------------------------
        /// Export the cumulative eigenvalues as a txt file
        ///
        /// @param[in] cumEigenvalues   a scalarField of cumEigenvalues.
        /// @param[in] name             a fileName to indicate the name of the txt file.
        /// @param[in] sup              a boolean variable 1 if you want to export the cumEigenValues (in ITHACAOutput/supremizer) 0 elsewhere (Default is 0).
        ///
        static void exportcumEigenvalues(scalarField cumEigenvalues,  fileName name,
                                         bool sup = 0);

        //--------------------------------------------------------------------------
        /// @brief      Get the DEIM modes for a generic non linear function
        ///
        /// @param[in]  SnapShotsMatrix  The snapshots matrix
        ///
        /// @tparam     T                templated object for the snapshots matrix type
        ///
        /// @return     T                the POD modes
        ///
        template<typename T>
        static PtrList<GeometricField<T, fvPatchField, volMesh>> DEIMmodes(
                    PtrList<GeometricField<T, fvPatchField, volMesh>>& SnapShotsMatrix, int nmodes,
                    word FunctionName);

        //--------------------------------------------------------------------------
        /// @brief      Get the DEIM modes for a generic non a parametrized matrix coming from a differential operator function
        ///
        /// @param[in]  MatrixList   The matrix list as a list of pointers
        /// @param[in]  nmodes       The number of modes
        /// @param[in]  MatrixName   The matrix name, used to save the matrix
        ///
        /// @tparam     the type of matrix, can be fvScalarMatrix or fvVectorMatrix
        ///
        /// @return     it returns a tuple containing the list of modes for the matrix and the list of modes for the source term
        ///
        template<typename type_matrix>
        static std::tuple<List<Eigen::SparseMatrix<double>>, List<Eigen::VectorXd>>
                DEIMmodes(PtrList<type_matrix>& MatrixList, int nmodesA, int nmodesB,
                          word MatrixName = "Matrix");

        //--------------------------------------------------------------------------
        /// @brief      Get the DEIM modes for a generic non a parametrized matrix coming from a differential operator function
        ///
        /// @param[in]  MatrixList   The matrix list as a list of pointers
        /// @param[in]  nmodes       The number of modes
        /// @param[in]  MatrixName   The matrix name, used to save the matrix
        ///
        /// @tparam     the type of matrix, can be fvScalarMatrix or fvVectorMatrix
        ///
        /// @return     it returns a tuple containing the list of modes for the matrix and the list of modes for the source term
        ///
        static std::tuple<List<Eigen::SparseMatrix<double>>, List<Eigen::VectorXd>>
                DEIMmodes(List<Eigen::SparseMatrix<double>>& A, List<Eigen::VectorXd>& b,
                          int nmodesA, int nmodesB, word MatrixName = "Matrix");


        //--------------------------------------------------------------------------
        /// @brief      Gets the modes.
        ///
        /// @param      snapshots   The snapshots
        /// @param      modes       The modes
        /// @param      Volumes     The volumes
        /// @param[in]  podex       The podex
        /// @param[in]  supex       The supex
        /// @param[in]  sup         The sup
        /// @param[in]  nmodes      The nmodes
        ///
        /// @tparam     Field_type  Type of field
        ///
        template<class Field_type>
        static void getModes(PtrList<Field_type>& snapshots, PtrList<Field_type>& modes,
                             PtrList<volScalarField>& Volumes, bool podex, bool supex = 0, bool sup = 0,
                             int nmodes = 0);

};

template<>
Eigen::MatrixXd ITHACAPOD::corMatrix(PtrList<volVectorField>& snapshots);

template<>
Eigen::MatrixXd ITHACAPOD::corMatrix(PtrList<volScalarField>& snapshots);

template<>
Eigen::MatrixXd ITHACAPOD::corMatrix(List<Eigen::SparseMatrix<double>>&
                                     snapshots);

template<>
Eigen::MatrixXd ITHACAPOD::corMatrix(List<Eigen::VectorXd>& snapshots);

template<>
void ITHACAPOD::exportBases(PtrList<volVectorField>& snapshots,
                            PtrList<volVectorField>& bases, bool sup);

template<>
void ITHACAPOD::exportBases(PtrList<volScalarField>& snapshots,
                            PtrList<volScalarField>& bases, bool sup);

template<>
void ITHACAPOD::normalizeBases(PtrList<volVectorField>& Bases);

template<>
void ITHACAPOD::normalizeBases(PtrList<volScalarField>& Bases);








// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif



