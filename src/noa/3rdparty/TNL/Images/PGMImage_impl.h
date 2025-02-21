// Copyright (c) 2004-2022 Tomáš Oberhuber et al.
//
// This file is part of TNL - Template Numerical Library (https://tnl-project.org/)
//
// SPDX-License-Identifier: MIT

#pragma once 

#include <cstring>
#include <noa/3rdparty/TNL/Images//PGMImage.h>

namespace noa::TNL {
namespace Images {   

template< typename Index >
PGMImage< Index >::
PGMImage() :
   binary( false ), maxColors( 0 ), fileOpen( false )
{
}

template< typename Index >
bool
PGMImage< Index >::
readHeader()
{
   std::string magicNumber;
   this->file >> magicNumber;
   if( this->file.fail() )
   {
      std::cerr << "Unable to read the magic number." << std::endl;
      return false;
   }

   if( magicNumber != "P5" && magicNumber != "P2" )
      return false;
 
   if(  magicNumber == "P5" )
      this->binary = true;

   char character;
   this->file.get(character);
   while ( ! this->file.eof() and ( character == ' ' || character == '\t' || character == '\r' || character == '\n') )
   {
	this->file.get(character);
	if ( character == '#' )
		while (! this->file.eof() && ( character != '\n' ) )
			this->file.get( character );
   }
   this->file.unget();
 
   this->file >> this->width >> this->height >> this->maxColors;
   return true;
}

template< typename Index >
bool
PGMImage< Index >::
openForRead( const String& fileName )
{
   this->close();
   if ( this->binary )
   	this->file.open( fileName.getString(), std::fstream::in | std::fstream::binary);
   else
	this->file.open( fileName.getString(), std::fstream::in );
   if( ! this->file )
   {
      std::cerr << "Unable to open the file " << fileName << std::endl;
      return false;
   }
   this->fileOpen = true;
   if( ! readHeader() )
      return false;
   return true;
}

template< typename Index >
   template< typename MeshReal,
             typename Device,
             typename Real >
bool
PGMImage< Index >::
read( const RegionOfInterest< Index > roi,
      Functions::MeshFunction< Meshes::Grid< 2, MeshReal, Device, Index >, 2, Real >& function )
{
   typedef Meshes::Grid< 2, Real, Device, Index > GridType;
   const GridType& grid = function.getMesh();
   typename GridType::Cell cell( grid );

   Index i, j;
   for( i = 0; i < this->height; i ++ )
      for( j = 0; j < this->width; j ++ )
      {
         int col;
         unsigned char col_aux;
         if( this->binary )
         {
           this->file >> col_aux;
           col = (int)col_aux;
         }
         else this->file >> col;
         if( roi.isIn( i, j ) )
         {
            cell.getCoordinates().x() = j - roi.getLeft();
            cell.getCoordinates().y() = roi.getBottom() - 1 - i;
            cell.refresh();
            function.getData().setElement( cell.getIndex(), ( Real ) col / ( Real ) this->maxColors );
         }
      }
   return true;
}

template< typename Index >
   template< typename Real,
             typename Device >
bool
PGMImage< Index >::
writeHeader( const Meshes::Grid< 2, Real, Device, Index >& grid,
             bool binary )
{
   if( binary )
      this->file << "P5\n";
   else
      this->file << "P2\n";
   this->file << "# This file was generated by TNL (tnl-image-converter)\n";
   this->file << grid.getDimensions().x() << ' '<< grid.getDimensions().y() << '\n' << "255\n";
   return true;
}

template< typename Index >
   template< typename Real,
             typename Device >
bool
PGMImage< Index >::
openForWrite( const String& fileName,
              Meshes::Grid< 2, Real, Device, Index >& grid,
              bool binary )
{
   this->close();
   if( binary )
        this->file.open( fileName.getString(), std::fstream::out | std::fstream::binary);
   else
        this->file.open( fileName.getString(), std::fstream::out);
   if( this->file.fail() )
   {
      std::cerr << "Unable to open the file " << fileName << std::endl;
      return false;
   }
   this->fileOpen = true;
   if( ! writeHeader( grid, binary ) )
      return false;
   this->binary = binary;
   return true;
}

template< typename Index >
   template< typename Real,
             typename Device,
             typename Vector >
bool
PGMImage< Index >::
write( const Meshes::Grid< 2, Real, Device, Index >& grid,
       Vector& vector )
{
   typedef Meshes::Grid< 2, Real, Device, Index > GridType;
   typename GridType::Cell cell( grid );
 
   Index i, j;
   for( i = 0; i < grid.getDimensions().y(); i ++ )
   {
      for( j = 0; j < grid.getDimensions().x(); j ++ )
      {
         cell.getCoordinates().x() = j;
         cell.getCoordinates().y() = grid.getDimensions().y() - 1 - i;
         cell.refresh();
 
         //Index cellIndex = grid.getCellIndex( CoordinatesType( j,
         //                                     grid.getDimensions().y() - 1 - i ) );

         unsigned char color = 255 * vector.getElement( cell.getIndex() );
         if ( ! this->binary )
         {
             int color_aux = (int)color;
             this->file << color_aux;
                  this->file << ' ';
         }
         else this->file << color;
      }      
      if ( ! this->binary )
         this->file << '\n';
   }
   return true;
}

template< typename Index >
   template< typename MeshReal,
             typename Device,
             typename Real >
bool
PGMImage< Index >::
write( const Functions::MeshFunction< Meshes::Grid< 2, MeshReal, Device, Index >, 2, Real >& function )
{
   typedef Meshes::Grid< 2, Real, Device, Index > GridType;
   const GridType& grid = function.getMesh();
   typename GridType::Cell cell( grid );
 
   Index i, j;
   for( i = 0; i < grid.getDimensions().y(); i ++ )
   {
      for( j = 0; j < grid.getDimensions().x(); j ++ )
      {
         cell.getCoordinates().x() = j;
         cell.getCoordinates().y() = grid.getDimensions().y() - 1 - i;
         cell.refresh();
 
         //Index cellIndex = grid.getCellIndex( CoordinatesType( j,
         //                                     grid.getDimensions().y() - 1 - i ) );

         unsigned char color = 255 * function.getData().getElement( cell.getIndex() );
         if ( ! this->binary )
         {
             int color_aux = (int)color;
             this->file << color_aux;
                  this->file << ' ';
         }
         else this->file << color;
      }      
      if ( ! this->binary )
         this->file << '\n';
   }
   return true;
}


template< typename Index >
void
PGMImage< Index >::
close()
{
   if( this->fileOpen )
      this->file.close();
   this->fileOpen = false;
}

template< typename Index >
PGMImage< Index >::
~PGMImage()
{
   close();
}

} // namespace Images
} // namespace noa::TNL

