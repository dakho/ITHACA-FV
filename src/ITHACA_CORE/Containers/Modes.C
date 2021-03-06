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

\*---------------------------------------------------------------------------*/


/// \file
/// Source file of the Modes class.

#include "Modes.H"

template<class T>
List<Eigen::MatrixXd> Modes<T>::toEigen()
{
    NBC = 0;

    for (int i = 0; i < (this->first()).boundaryFieldRef().size(); i++)
    {
        if ((this->first()).boundaryFieldRef()[i].type() != "processor")
        {
            NBC++;
        }
    }

    EigenModes.resize(NBC + 1);
    EigenModes[0] = Foam2Eigen::PtrList2Eigen(this->toPtrList());
    List<Eigen::MatrixXd> BC = Foam2Eigen::PtrList2EigenBC(this->toPtrList());

    for (int i = 0; i < NBC; i++)
    {
        EigenModes[i + 1] = BC[i];
    }

    return EigenModes;
}

template<class T>
List<Eigen::MatrixXd> Modes<T>::project(fvMatrix<T>& Af, int numberOfModes)
{
    List<Eigen::MatrixXd> LinSys;
    LinSys.resize(2);

    if (EigenModes.size() == 0)
    {
        toEigen();
    }

    Eigen::SparseMatrix<double> Ae;
    Eigen::VectorXd be;
    Foam2Eigen::fvMatrix2Eigen(Af, Ae, be);

    if (numberOfModes == 0)
    {
        LinSys[0] = EigenModes[0].transpose() * Ae * EigenModes[0];
        LinSys[1] = EigenModes[0].transpose() * be;
    }
    else
    {
        M_Assert(numberOfModes <= EigenModes[0].cols(),
                 "Number of required modes for projection is higher then the number of available ones");
        LinSys[0] = ((EigenModes[0]).leftCols(numberOfModes)).transpose() * Ae *
                    (EigenModes[0]).leftCols(numberOfModes);
        LinSys[1] = ((EigenModes[0]).leftCols(numberOfModes)).transpose() * be;
    }

    return LinSys;
}


template<class T>
GeometricField<T, fvPatchField, volMesh> Modes<T>::reconstruct(
    Eigen::MatrixXd& Coeff,
    word Name)
{
    if (EigenModes.size() == 0)
    {
        toEigen();
    }

    GeometricField<T, fvPatchField, volMesh> field(Name,
            (this->toPtrList())[0] * 0);
    int Nmodes = Coeff.rows();
    Eigen::VectorXd InField = EigenModes[0].leftCols(Nmodes) * Coeff;
    field = Foam2Eigen::Eigen2field(field, InField);

    for (int i = 0; i < NBC; i++)
    {
        Eigen::VectorXd BF = EigenModes[i + 1].leftCols(Nmodes) * Coeff;
        ITHACAutilities::assignBC(field, i, BF);
    }

    return field;
}

template<class T>
PtrList<GeometricField<T, fvPatchField, volMesh>>
        Modes<T>::projectSnapshots(
            PtrList<GeometricField<T, fvPatchField, volMesh>> snapshots,
            PtrList<volScalarField> Volumes,
            int numberOfModes,
            word innerProduct)
{
    if (EigenModes.size() == 0)
    {
        toEigen();
    }

    M_Assert(snapshots.size() == Volumes.size(),
             "The number of snapshots and the number of volumes vectors must be equal");
    M_Assert(numberOfModes <= this->size(),
             "The number of Modes used for the projection cannot be bigger than the number of available modes");
    M_Assert(innerProduct == "L2" || innerProduct == "Frobenius",
             "The chosen inner product is not implemented");
    int dim = std::nearbyint(EigenModes[0].rows() /
                             Volumes[0].size()); //Checking if volumes and modes have the same size that means check if the problem is vector or scalar
    Eigen::MatrixXd totVolumes(Volumes[0].size()*dim, Volumes.size());

    for (int i = 0; i < Volumes.size(); i++)
    {
        totVolumes.col(i) = Foam2Eigen::field2Eigen(Volumes[i]);
    }

    totVolumes.replicate(dim, 1);
    Eigen::MatrixXd Modes;

    if (numberOfModes == 0)
    {
        Modes = EigenModes[0];
    }
    else
    {
        Modes = EigenModes[0].leftCols(numberOfModes);
    }

    Eigen::MatrixXd M;
    PtrList<GeometricField<T, fvPatchField, volMesh>> projSnap;
    Eigen::MatrixXd projSnapI;
    Eigen::MatrixXd projSnapCoeff;

    for (int i = 0; i < snapshots.size(); i++)
    {
        Eigen::MatrixXd F_eigen = Foam2Eigen::field2Eigen(snapshots[i]);

        if (innerProduct == "L2")
        {
            M = Modes.transpose() * (totVolumes.col(i)).asDiagonal() * Modes;
            projSnapI = Modes.transpose() * (totVolumes.col(i)).asDiagonal() * F_eigen;
        }
        else //Frobenius
        {
            M = Modes.transpose() * Modes;
            projSnapI = Modes.transpose() * F_eigen;
        }

        projSnapCoeff = M.fullPivLu().solve(projSnapI);
        projSnap.append(reconstruct(projSnapCoeff, "projSnap"));
    }

    return projSnap;
}

template<class T>
PtrList<GeometricField<T, fvPatchField, volMesh>>
        Modes<T>::projectSnapshots(
            PtrList<GeometricField<T, fvPatchField, volMesh>> snapshots,
            PtrList<volScalarField> Volumes, word innerProduct)
{
    int numberOfModes = 0;
    PtrList<GeometricField<T, fvPatchField, volMesh>> projSnap =  projectSnapshots(
                snapshots, Volumes, numberOfModes, innerProduct);
    return projSnap;
}

template<class T>
PtrList<GeometricField<T, fvPatchField, volMesh>>
        Modes<T>::projectSnapshots(
            PtrList<GeometricField<T, fvPatchField, volMesh>> snapshots,
            int numberOfModes,
            word innerProduct)
{
    M_Assert(numberOfModes <= this->size(),
             "The number of Modes used for the projection cannot be bigger than the number of available modes");
    M_Assert(innerProduct == "L2" || innerProduct == "Frobenius",
             "The chosen inner product is not implemented");
    Eigen::MatrixXd Modes;

    if (numberOfModes == 0)
    {
        Modes = EigenModes[0];
    }
    else
    {
        Modes = EigenModes[0].leftCols(numberOfModes);
    }

    if (EigenModes.size() == 0)
    {
        toEigen();
    }

    Eigen::MatrixXd M_vol;
    Eigen::MatrixXd M;
    PtrList<GeometricField<T, fvPatchField, volMesh>> projSnap;
    Eigen::MatrixXd projSnapI;
    Eigen::MatrixXd projSnapCoeff;

    for (int i = 0; i < snapshots.size(); i++)
    {
        Eigen::MatrixXd F_eigen = Foam2Eigen::field2Eigen(snapshots[i]);

        if (innerProduct == "L2")
        {
            M_vol = ITHACAutilities::get_mass_matrix_FV(snapshots[i]);
        }
        else if (innerProduct == "Frobenius")
        {
            M_vol =  Eigen::VectorXd::Identity(F_eigen.rows(), 1);
        }
        else
        {
            std::cout << "Inner product not defined" << endl;
            exit(0);
        }

        M = Modes.transpose() * M_vol.asDiagonal() * Modes;
        projSnapI = Modes.transpose() * M_vol.asDiagonal() * F_eigen;
        projSnapCoeff = M.fullPivLu().solve(projSnapI);
        projSnap.append(reconstruct(projSnapCoeff, "projSnap"));
    }

    return projSnap;
}

template<class T>
PtrList<GeometricField<T, fvPatchField, volMesh>>
        Modes<T>::projectSnapshots(
            PtrList<GeometricField<T, fvPatchField, volMesh>> snapshots, word innerProduct)
{
    int numberOfModes = 0;
    PtrList<GeometricField<T, fvPatchField, volMesh>> projSnap =  projectSnapshots(
                snapshots, numberOfModes, innerProduct);
    return projSnap;
}

template class Modes<scalar>;
template class Modes<vector>;


