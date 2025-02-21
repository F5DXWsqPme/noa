// Copyright (c) 2004-2022 Tomáš Oberhuber et al.
//
// This file is part of TNL - Template Numerical Library (https://tnl-project.org/)
//
// SPDX-License-Identifier: MIT

#pragma once

#include <noa/3rdparty/TNL/Assert.h>
#include <noa/3rdparty/TNL/Cuda/LaunchHelpers.h>
#include <noa/3rdparty/TNL/Containers/VectorView.h>
#include <noa/3rdparty/TNL/Algorithms/ParallelFor.h>
#include <noa/3rdparty/TNL/Algorithms/Segments/detail/LambdaAdapter.h>

namespace noa::TNL {
   namespace Algorithms {
      namespace Segments {

template< typename Index,
          typename Device >
struct CSRVectorKernel
{
   using IndexType = Index;
   using DeviceType = Device;
   using ViewType = CSRVectorKernel< Index, Device >;
   using ConstViewType = CSRVectorKernel< Index, Device >;

   template< typename Offsets >
   void init( const Offsets& offsets );

   void reset();

   ViewType getView();

   ConstViewType getConstView() const;

   static noa::TNL::String getKernelType();

   template< typename OffsetsView,
             typename Fetch,
             typename Reduction,
             typename ResultKeeper,
             typename Real,
             typename... Args >
   static void reduceSegments( const OffsetsView& offsets,
                                  Index first,
                                  Index last,
                                  Fetch& fetch,
                                  const Reduction& reduction,
                                  ResultKeeper& keeper,
                                  const Real& zero,
                                  Args... args );
};

      } // namespace Segments
   }  // namespace Algorithms
} // namespace noa::TNL

#include <noa/3rdparty/TNL/Algorithms/Segments/Kernels/CSRVectorKernel.hpp>
