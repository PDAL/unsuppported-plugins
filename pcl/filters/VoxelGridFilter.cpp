/******************************************************************************
* Copyright (c) 2015, Bradley J Chambers (brad.chambers@gmail.com)
*
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following
* conditions are met:
*
*     * Redistributions of source code must retain the above copyright
*       notice, this list of conditions and the following disclaimer.
*     * Redistributions in binary form must reproduce the above copyright
*       notice, this list of conditions and the following disclaimer in
*       the documentation and/or other materials provided
*       with the distribution.
*     * Neither the name of Hobu, Inc. or Flaxen Geo Consulting nor the
*       names of its contributors may be used to endorse or promote
*       products derived from this software without specific prior
*       written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
* FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
* COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
* INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
* BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
* OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
* AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
* OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
* OF SUCH DAMAGE.
****************************************************************************/

#include "VoxelGridFilter.hpp"

#include <pcl/console/print.h>
#include <pcl/point_types.h>
#include <pcl/io/pcd_io.h>
#include <pcl/filters/voxel_grid.h>
#include <pdal/EigenUtils.hpp>
#include <pdal/util/ProgramArgs.hpp>

#include "../PCLConversions.hpp"

namespace pdal
{

static PluginInfo const s_info
{
    "filters.voxelgrid",
    "Voxel grid filter",
    "http://pdal.io/stages/filters.voxelgrid.html"
};

CREATE_SHARED_STAGE(VoxelGridFilter, s_info)

std::string VoxelGridFilter::getName() const
{
    return s_info.name;
}


void VoxelGridFilter::addArgs(ProgramArgs& args)
{
    args.add("leaf_x", "Leaf size in X dimension", m_leaf_x, 1.0);
    args.add("leaf_y", "Leaf size in Y dimension", m_leaf_y, 1.0);
    args.add("leaf_z", "Leaf size in Z dimension", m_leaf_z, 1.0);
}

PointViewSet VoxelGridFilter::run(PointViewPtr input)
{
    PointViewPtr output = input->makeNew();
    PointViewSet viewSet;
    viewSet.insert(output);

    bool logOutput = log()->getLevel() > LogLevel::Debug1;
    if (logOutput)
        log()->floatPrecision(8);

    log()->get(LogLevel::Debug2) << "Process VoxelGridFilter..." << std::endl;

    BOX3D buffer_bounds;
    calculateBounds(*input, buffer_bounds);

    // convert PointView to PointNormal
    typedef pcl::PointCloud<pcl::PointXYZ> Cloud;
    Cloud::Ptr cloud(new Cloud);
    pclsupport::PDALtoPCD(input, *cloud, buffer_bounds);

    pclsupport::setLogLevel(log()->getLevel());

    // initial setup
    pcl::VoxelGrid<pcl::PointXYZ> vg;
    vg.setInputCloud(cloud);
    vg.setLeafSize(m_leaf_x, m_leaf_y, m_leaf_z);

    // create PointCloud for results
    Cloud::Ptr cloud_f(new Cloud);
    vg.filter(*cloud_f);

    if (cloud_f->points.empty())
    {
        log()->get(LogLevel::Debug2) << "Filtered cloud has no points!" << std::endl;
        return viewSet;
    }

    pclsupport::PCDtoPDAL(*cloud_f, output, buffer_bounds);

    log()->get(LogLevel::Debug2) << cloud->points.size() << " before, " <<
                                 cloud_f->points.size() << " after" << std::endl;
    log()->get(LogLevel::Debug2) << output->size() << std::endl;

    return viewSet;
}

} // namespace pdal
