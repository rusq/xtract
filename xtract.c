/* Copyright (C) 2000  Timothy Hale and Mathieu Olivier
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <stdio.h> // fseek, SEEK_SET, fflush, stdout, fclose, fread, printf, FILE, perror
#include <string.h> // strncmp, strstr
#include <process.h> // system
//#include <dir.h> // MAXPATH, MAXDRIVE, MAXDIR, MAXFILE, MAXEXT
//#include <conio.h> // kbhit, getch
//#include <malloc.h> //
#include <stdlib.h> // EXIT_SUCCESS, EXIT_FAILURE, _MAX_PATH, malloc, free
#include <direct.h> // _chdir, _mkdir, _getcwd
/* ----------------- */
/* --- Constants --- */
/* ----------------- */

// Maximum number of files in a group file: (taken from the original "kextract.c")
#define MAX_NB_FILES 4096
// Maximum number of tiles (taken from Ken's "editart.c")
#define MAX_NB_TILES 9216

// Size of the color palette (256 colors, 3 components for each color)
#define PALETTE_SIZE (256 * 3)

/* ------------- */
/* --- Types --- */
/* ------------- */

// Boolean
#ifndef __cplusplus  // Standard C++ has already this "bool"
   typedef enum { false, true } bool;
#endif

// Properties of a grouped file
typedef struct {
   char         Name [13];            // 8.3 format, zero-terminated
   bool         MarkedForExtraction;
   unsigned int Length;
   unsigned int Offset;
} FileProperties_t;

// Description of a tile
typedef struct {
   unsigned short XSize;   // Tile's width
   unsigned short YSize;   // Tile's height
   unsigned int Offset;  // Offset in the ART file
} Tile_t;

    // ART file
    FILE* ArtFile = NULL;
    char ArtFileName[13];
    // List of tiles in the ART file
    unsigned int NbTiles = 0;
    unsigned int TilesStartNum;             // Number of the first tile
    Tile_t TilesList [MAX_NB_TILES];

    // Color palette (in TGA format: set of {Blue, Green, Red})
    unsigned char Palette [PALETTE_SIZE];

/* -------------- */
/* --- Macros --- */
/* -------------- */

// If the OS don't have the "min" macro, define it
#ifndef min
   // Return the minimum value of 2 values
   #define min(a,b) (((a) < (b)) ? (a) : (b))
#endif


/* ------------------------- */
/* --- Globals variables --- */
/* ------------------------- */

// Properties of the grouped files
static FileProperties_t FilesProperties [MAX_NB_FILES];

// Total number of files in the group file, and number of files to extract
static unsigned int NbFiles, NbFilesToExtract;


/* ----------------- */
/* --- Functions --- */
/* ----------------- */

/* ------ Prototypes ------ */

// Mark the files to extract
static void MarkFiles (const char* FileSpec);

// Extract pictures from the ART file
static bool ExtractPictures (void);

// Get a unsigned short from a little-endian ordered buffer
static unsigned short GetLittleEndianUInt16 (unsigned char* Buffer);

// Get a unsigned int from a little-endian ordered buffer
static unsigned int GetLittleEndianUInt32 (unsigned char* Buffer);

// Create the pictures list from the ART header
static bool GetPicturesList (void);

// Load the color palette from the PALETTE.DAT file
static bool LoadPalette (void);

// Set a unsigned short into a little-endian ordered buffer
static void SetLittleEndianUInt16 (unsigned short Integer, unsigned char* Buffer);

// Extract the picture at TilesList[TileInd] and save it as PictureName
static bool SpawnTGA (unsigned int TileInd, const char* PictureName);

// Starts the whole art to targa process
static bool DecompileArt(char * ArtFileName);


/* ------ Implementations ------ */

/*
====================
main

Main procedure
====================
*/
int main (int ArgC, char* ArgV [])
{
   // Variables
   FILE *GroupFile, *CrtFile;
   unsigned char Buffer [64 * 1024];
   unsigned int Ind, Ind2;
   unsigned int DataSize;
   unsigned int CrtOffset;
   const char* FileName;
   char temp[_MAX_PATH], buffer[_MAX_PATH];

   // Header
   printf ("\n"
           "Xtract version 0.0 by Timothy Hale\n"
           "   based largely on code by Mathieu Olivier\n"
           "============================================\n\n"
          );

   // Check arguments
   if (ArgC < 2)
   {
      printf ("Syntax: Xtract [grouped file]\n"
              "   This program extracts files from a previously grouped group file.\n"
              "   Ex: Xtract stuff.dat\n (stuff.dat is the group file)\n"
             );
      return -1;
   }

   // Open the group file
   FileName = ArgV[1];

   _mkdir ("tiles");
   _mkdir ("maps");

   GroupFile = fopen (FileName, "rb");
   if (GroupFile == NULL)
   {
      printf ("Error: %s could not be opened\n", FileName);
      return -1;
   }

   // Check the group file tag (12 first bytes must be "KenSilverman")
   if (fread (Buffer, 1, 16, GroupFile) != 16 ||
       strncmp ("KenSilverman", (char*)Buffer, 12) != 0)
   {
      fclose (GroupFile);
      printf ("Error: %s not a valid group file\n", FileName);
      return -1;
   }

   // Read the grouped files properties
   NbFiles = Buffer[12] | Buffer[13] << 8 | Buffer[14] << 16 | Buffer[15] << 24;
   CrtOffset = 16 + NbFiles * 16;  // base offset, just after the header
   for (Ind = 0; Ind < NbFiles; Ind++)
   {
      // Extract the name of the file
      fread (FilesProperties[Ind].Name, 1, 12, GroupFile);
      FilesProperties[Ind].Name[12] = '\0';  // "close" the name manually if it was too long (12 chars)

      // Extract the length of the file
      fread (Buffer, 1, 4, GroupFile);
      FilesProperties[Ind].Length = Buffer[0] | Buffer[1] << 8 | Buffer[2] << 16 | Buffer[3] << 24;

      // Assign the current offset to the file
      FilesProperties[Ind].Offset = CrtOffset;

      // Unmark file for extraction
      FilesProperties[Ind].MarkedForExtraction = false;

      // Compute the new current offset
      CrtOffset += FilesProperties[Ind].Length;
   }

      MarkFiles ("palette.dat");
    //  MarkFiles ("*.map");
      MarkFiles ("*.art");


   // If there's no file to extract, we quit
   if (NbFilesToExtract == 0)
   {
      fclose (GroupFile);
      printf ("No map files found in group file\n");
      return 0;
   }

   // Extract the chosen files
   for (Ind = 0; Ind < NbFiles; Ind++)
   {
      // If the file doesn't have to be extracted, skip it
      if (FilesProperties[Ind].MarkedForExtraction == 0)
         continue;

      // Seek the file in the group file
      if (fseek (GroupFile, FilesProperties[Ind].Offset, SEEK_SET) != 0)
      {
         printf ("Error: %s has an invalid offset\n", FilesProperties[Ind].Name);
         continue;
      }

      // Create the file
      CrtFile = fopen (FilesProperties[Ind].Name, "wb");
      if (CrtFile == NULL)
      {
         printf ("Error: Could not create %s\n", FilesProperties[Ind].Name);
         continue;
      }

      // Fill it
      printf ("\nExtracting %s... ", FilesProperties[Ind].Name);
      fflush (stdout);
      for (Ind2 = 0; Ind2 < FilesProperties[Ind].Length; Ind2 += sizeof (Buffer))
      {
         // Read the data from the group file
         DataSize = min (FilesProperties[Ind].Length - Ind2, sizeof (Buffer));
         if (fread (Buffer, 1, DataSize, GroupFile) != DataSize)
         {
            printf ("error (can't read the whole file)\n");
            break;
         }

         // Write the data to the new created file
         if (fwrite (Buffer, 1, DataSize, CrtFile) != DataSize)
         {
            printf ("error (can't write the whole file)\n");
            fclose (CrtFile);
            fclose (GroupFile);
            return -1;
         }
      } // for (Ind2 = 0; Ind2 < FilesProperties[Ind].Length; Ind2 += sizeof (Buffer))
      printf ("done\n");
      fclose (CrtFile);

      /* Get the current working directory: */
      if( _getcwd( buffer, _MAX_PATH ) == NULL )
      {
          perror( "_getcwd error" );
          return EXIT_FAILURE;
      }

      // If it's a map file chunk
      if (strstr (FilesProperties[Ind].Name, ".MAP") !=NULL)
      {
      sprintf(temp, "%s%s%s%s%s%s", "dukeconv ", FilesProperties[Ind].Name, " ", buffer, "\\maps",
               FilesProperties[Ind].Name);
      printf(temp,"\n");
      system(temp);
      printf("\n");
      }

      // If it's an art file chunk
      if (strstr (FilesProperties[Ind].Name, ".ART") !=NULL)
      {
            sprintf(ArtFileName,"%s",FilesProperties[Ind].Name);

            DecompileArt(ArtFileName);

      }
   }

   // to clean up any dummy old school files
   for (Ind = 0; Ind < NbFiles; Ind++)
   {
       if (strstr (FilesProperties[Ind].Name, ".MAP") !=NULL)
       {
           sprintf(temp, "%s%s", "del ", FilesProperties[Ind].Name);
           system(temp);
       }

       if (strstr (FilesProperties[Ind].Name, ".ART") !=NULL)
       {
           sprintf(temp, "%s%s", "del ", FilesProperties[Ind].Name);
           system(temp);
       }

   }

   // Finalization
   fclose (GroupFile);
   printf ("\n");


   return 0;
}


/*
====================
MarkFiles

Mark the files to extract
====================
*/
static void MarkFiles (const char* FileSpec)
{
   // Variables
   unsigned int Ind, Ind2, Ind3;
   char LocalFileSpec [12], LocalFileName [12];
   char SpecChar, NameChar;
   bool Matched;

   // Put the FileSpec in LocalFileSpec with a format on 12 characters ("<prefix>_SPACES_.<suffix>")
   Ind2 = 0;
   for (Ind = 0; FileSpec[Ind] != '\0'; Ind++)
   {
      // if we find the suffix, move forward to its place in LocalFileSpec
      // and fill the space between prefic and suffix with spaces
      if (FileSpec[Ind] == '.')
         while (Ind2 < 8)
            LocalFileSpec[Ind2++] = ' ';

      // Copy the current character in LocalFileSpec
      LocalFileSpec[Ind2++] = FileSpec[Ind];
   }

   // Match LocalFileSpec with FileName for each file in the group file
   for (Ind = 0; Ind < NbFiles; Ind++)
   {
      // Put the FileProperties[Ind].Name in LocalFileName with the same format on 12 characters
      Ind2 = 0;
      for (Ind3 = 0; FilesProperties[Ind].Name[Ind3] != '\0'; Ind3++)
      {
         // if we find the suffix, move forward to its place in LocalFileSpec
         // and fill the space between prefic and suffix with spaces
         if (FilesProperties[Ind].Name[Ind3] == '.')
            while (Ind2 < 8)
               LocalFileName[Ind2++] = ' ';

         // Copy the current character in LocalFileName
         LocalFileName[Ind2++] = FilesProperties[Ind].Name[Ind3];
      }

      // Matching...
      Matched = true;
      for (Ind2 = 0; Ind2 < sizeof (LocalFileSpec); Ind2++)
      {
         // Extract the 2 current characters and "upcase" them
         SpecChar = LocalFileSpec[Ind2];
         if (SpecChar >= 'a' && SpecChar <= 'z')
            SpecChar -= 32;
         NameChar = LocalFileName[Ind2];
         if (NameChar >= 'a' && NameChar <= 'z')
            NameChar -= 32;

         // If it's the '*' joker
         if (SpecChar == '*')
         {
            // If we are in the suffix part, that's OK
            if (Ind2 >= 8)
               break;

            // Else, go to the suffix part
            Ind2 = 8;
         }

         // Else, if the 2 characters can't be matched, we skip this file
         else if (SpecChar != '?' && SpecChar != NameChar)
         {
            Matched = false;
            break;
         }
      }

      // If this file must be extracted, mark it
      if (Matched)
      {
         FilesProperties[Ind].MarkedForExtraction = true;
         NbFilesToExtract++;
      }
   }
}

static bool DecompileArt(char* ArtFileName)
{

   // Load the color palette
   if (! LoadPalette ())
      return EXIT_FAILURE;

   // Open the ART file
   ArtFile = fopen (ArtFileName, "rb");
   if (ArtFile == NULL)
   {
      printf ("Error: can't open file \"%s\"\n", ArtFileName);
      return EXIT_FAILURE;
   }

   // Read the ART header
   if (! GetPicturesList ())
   {
      fclose (ArtFile);
      return EXIT_FAILURE;
   }

   // Extract pictures from the ART file
   if (! ExtractPictures ())
      return EXIT_FAILURE;

   fclose (ArtFile);
   return EXIT_SUCCESS;
}

/*
====================
LoadPalette

Load the color palette from the PALETTE.DAT file
====================
*/
static bool LoadPalette (void)
{
   // Variables
   FILE* PaletteFile;
   unsigned int Ind;
   unsigned char Color;

   // Open the file
   PaletteFile = fopen ("palette.dat", "rb");
   if (PaletteFile == NULL)
   {
      printf ("Error: can't open \"palette.dat\"\n");
      return false;
   }

   // Read the palette
   if (fread (Palette, 1, PALETTE_SIZE, PaletteFile) != PALETTE_SIZE)
   {
      printf ("Error: can't read the whole palette from \"palette.dat\" file\n");
      fclose (PaletteFile);
      return false;
   }

   // Convert it to TGA palette format
   for (Ind = 0; Ind < PALETTE_SIZE; Ind += 3)
   {
      Color = Palette[Ind];
      Palette[Ind] = (unsigned char)(Palette[Ind + 2] << 2);
      Palette[Ind + 1] <<= 2;
      Palette[Ind + 2] = (unsigned char)(Color << 2);
   }

   printf ("Palette successfully loaded\n");
   fclose (PaletteFile);
   return true;
}

/*
====================
GetPicturesList

Create the pictures list from the ART header
====================
*/
static bool GetPicturesList (void)
{
   // Variables
   unsigned char Buffer [MAX_NB_TILES * 4];
   unsigned int Version, TilesEndNum;
   unsigned int Ind;
   size_t CrtOffset;

   // Read the first part of the header
   if (fread (Buffer, 1, 16, ArtFile) != 16)
   {
      printf ("Error: invalid ART file: not enough header data\n");
      return false;
   }
   Version       = GetLittleEndianUInt32 (&Buffer[ 0]);
   NbTiles       = GetLittleEndianUInt32 (&Buffer[ 4]); // useless; don't use it
   TilesStartNum = GetLittleEndianUInt32 (&Buffer[ 8]);
   TilesEndNum   = GetLittleEndianUInt32 (&Buffer[12]);

   // Compute the real number of tiles contained in the file
   NbTiles = TilesEndNum - TilesStartNum + 1;

   // Check the version number
   if (Version != 1)
   {
      printf ("Error: invalid ART file: invalid version number (%u)\n", Version);
      return false;
   }

   printf ("%u tiles declared in the ART header\n", NbTiles);

   // Extract sizes
   fread (Buffer, 1, NbTiles * 2, ArtFile);
   for (Ind = 0; Ind < NbTiles; Ind++)
      TilesList[Ind].XSize = GetLittleEndianUInt16 (&Buffer[Ind * 2]);
   fread (Buffer, 1, NbTiles * 2, ArtFile);
   for (Ind = 0; Ind < NbTiles; Ind++)
      TilesList[Ind].YSize = GetLittleEndianUInt16 (&Buffer[Ind * 2]);


   // Compute offsets (useless for now)
   CrtOffset = 16 + NbTiles * (2 + 2 + 4);
   for (Ind = 0; Ind < NbTiles; Ind++)
   {
      TilesList[Ind].Offset = CrtOffset;
      CrtOffset += TilesList[Ind].XSize * TilesList[Ind].YSize;
   }

   // ... Should use animation flags ...

   return true;
}

/*
====================
ExtractPictures

Extract pictures from the ART file
====================
*/
static bool ExtractPictures (void)
{
   // Variables
   unsigned int Ind;
   char ImageFileName [13];

   for (Ind = 0; Ind < NbTiles; Ind++)
   {
      sprintf (ImageFileName, "%03d-%c%c%c.tga", Ind,
          ArtFileName[5], ArtFileName[6], ArtFileName[7]);
      SpawnTGA (Ind, ImageFileName);
   }

   printf ("\n");
   return true;
}

/*
====================
GetLittleEndianUInt16

Get a unsigned short from a little-endian ordered buffer
====================
*/
static unsigned short GetLittleEndianUInt16 (unsigned char* Buffer)
{
   return (unsigned short)(Buffer[0] | (Buffer[1] << 8));
}


/*
====================
GetLittleEndianUInt32

Get a unsigned int from a little-endian ordered buffer
====================
*/
static unsigned int GetLittleEndianUInt32 (unsigned char* Buffer)
{
   return Buffer[0] | (Buffer[1] << 8) | (Buffer[2] << 16) | (Buffer[3] << 24);
}

/*
====================
SpawnTGA

Extract the picture number "PictureInd" and save it as PictureName
====================
*/
static bool SpawnTGA (unsigned int TileInd, const char* PictureName)
{
   // Variables
   FILE* ImageFile;
   int XInd, YInd;
   unsigned char TgaHeader [] = {
      0x00,                          // Number of Characters in Identification Field (0)
      0x01,                          // Color Map Type (true)
      0x01,                          // Image Type Code (1 -> uncompressed color mapped)
      0x00, 0x00, 0x00, 0x01, 0x18,  // Color Map Specification (256 colors of 24 bits, starting from 0)
      0x00, 0x00, 0x00, 0x00,        // Origin of Image (0, 0)
      0x00, 0x00, 0x00, 0x00,        // Size of Image (TO BE FILLED. Bytes 12-13 and 14-15. little-endian)
      0x08,                          // Pixel size (8 bits)
      0x00                           // Flags (image described from bottom left; no alpha)
   };
   unsigned char* ImageBuffer;
   const unsigned int PictureSize = TilesList[TileInd].XSize * TilesList[TileInd].YSize;
   char buffer[_MAX_PATH], temp[_MAX_PATH];

   /* Get the current working directory: */
   if( _getcwd( buffer, _MAX_PATH ) == NULL )
      perror( "_getcwd error" );
   else
      sprintf(temp, "%s%s", buffer, "\\tiles");

   // If the picture is empty, we quit
   if (PictureSize == 0)
   {
      printf ("Warning: \"%s\" will not be extracted because it's an empty tile\n", PictureName);
      return true;
   }

   // Load the picture in a buffer
   fseek (ArtFile, TilesList[TileInd].Offset, SEEK_SET);
   ImageBuffer = malloc (PictureSize);
   if (ImageBuffer == NULL)
   {
      printf ("Error: can't allocate enough memory to load \"%s\"\n", PictureName);
      return false;
   }

   if (fread (ImageBuffer, 1, PictureSize, ArtFile) != PictureSize)
   {
      printf ("Error: can't read enough data in the ART file to load \"%s\"\n", PictureName);
      free (ImageBuffer);
      return false;
   }


   if(_chdir (temp))
   {
      perror("Error changing directory to tiles\n");
      return false;
   }

   // Create the file
   ImageFile = fopen (PictureName, "wb");
   if (ImageFile == NULL)
   {
      printf ("Error: can't create file \"%s\"\n", PictureName);
      free (ImageBuffer);
      return false;
   }

   // Write the TGA header (including the palette)
   SetLittleEndianUInt16 (TilesList[TileInd].XSize, &TgaHeader[12]);
   SetLittleEndianUInt16 (TilesList[TileInd].YSize, &TgaHeader[14]);
   fwrite (TgaHeader, 1, sizeof (TgaHeader), ImageFile);
   fwrite (Palette, 1, PALETTE_SIZE, ImageFile);

   // Write the picture
   for (YInd = TilesList[TileInd].YSize - 1; YInd >= 0; YInd--)  // save the lines from bottom
      for (XInd = 0; XInd < TilesList[TileInd].XSize; XInd++)
         // ART file store pictures by columns, not by lines
         fwrite (&ImageBuffer[YInd + XInd * TilesList[TileInd].YSize], 1, 1, ImageFile);

   free (ImageBuffer);
   fclose (ImageFile);


   if(_chdir (buffer))
   {
      printf("Error going up a directory\n");
      return false;
   }

   return true;
}

/*
====================
SetLittleEndianUInt16

Set a unsigned short into a little-endian ordered buffer
====================
*/
static void SetLittleEndianUInt16 (unsigned short Integer, unsigned char* Buffer)
{
   Buffer[0] = (unsigned char)(Integer & 255);
   Buffer[1] = (unsigned char)(Integer >> 8);
}
