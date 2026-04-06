///
///    Load an image
///
SDL_Surface *LoadBitmap(char *resourcefilename, char *bitmapfilename)
{
  //Get the bitmap's buffer and size from the resource file
	int filesize = 0;
	char *buffer = GetBufferFromResource(resourcefilename, bitmapfilename, &filesize);

	//Load the buffer into a surface using SDL3 I/O
	SDL_IOStream *io = SDL_IOFromMem(buffer, filesize);
	SDL_Surface *temp = SDL_LoadBMP_IO(io, true);

	//Release the bitmap buffer memory
	free(buffer);

	//Were we able to load the bitmap?
	if (temp == NULL)
	{
		printf("Unable to load bitmap: %s\n", SDL_GetError());
		exit(1);
	}

	//SDL3 no longer uses SDL_DisplayFormat; return the loaded surface as-is
	return temp;
}

///
///    Load a sound using Mix_Chunk
///
Mix_Chunk *LoadSound(char *resourcefilename, char *soundfilename)
{
	//Get the sound's buffer and size from the resource file
	int filesize = 0;
	char *buffer = GetBufferFromResource(resourcefilename, soundfilename, &filesize);

	//Load the buffer into a chunk using SDL3 I/O
	SDL_IOStream *io = SDL_IOFromMem(buffer, filesize);
	Mix_Chunk *sound = Mix_LoadWAV_IO(io, true);

	//Release the buffer memory
	free(buffer);

	//Return the sound
	return sound;
}

///
///    Load a music using Mix_Music
///
Mix_Music *LoadMusic(char *resourcefilename, char *soundfilename)
{
	//Get the sound's buffer and size from the resource file
	int filesize = 0;
	char *buffer = GetBufferFromResource(resourcefilename, soundfilename, &filesize);

	//Load the buffer into music using SDL3 I/O
	SDL_IOStream *io = SDL_IOFromMem(buffer, filesize);
	Mix_Music *sound = Mix_LoadMUS_IO(io, true);

	//You can't free the buffer, otherwise the App crashes
	//free(buffer);

	//Return the sound
	return sound;
}
