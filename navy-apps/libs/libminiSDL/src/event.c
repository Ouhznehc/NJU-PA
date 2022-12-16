#include <NDL.h>
#include <SDL.h>
#include <string.h>
#define keyname(k) #k,
#define key_offset 9
static const char *keyname[] = {
  "NONE",
  _KEYS(keyname)
};
static uint8_t keystate[sizeof(keyname) / sizeof(char *)];

int SDL_PushEvent(SDL_Event *ev) {
  return 0;
}

int SDL_PollEvent(SDL_Event *ev) {
  char key_buf[64], key_name[64];
  int key_index = 0;
  if(!NDL_PollEvent(key_buf, sizeof(key_buf))) return 0; // return intsantly
  if(key_buf[4] == 'd')ev->type = SDL_KEYDOWN; // key down xxx
  else ev->type = SDL_KEYUP; // key up   xxx
  for(key_index = 0; key_buf[key_index + key_offset] != '\n'; key_index++)
    key_name[key_index] = key_buf[key_index + key_offset];
  key_name[key_index] = '\0';

  for(int i = 0; i < sizeof(keyname) / sizeof(char *); i++)
    if (strcmp(key_name, keyname[i]) == 0){
      ev->key.keysym.sym = i;
      keystate[i] = ~ev->type;
    }
  return 1;
}

int SDL_WaitEvent(SDL_Event *event) {
  char key_buf[64], key_name[64];
  int ret = 0, key_index = 0;
  while(!NDL_PollEvent(key_buf, sizeof(key_buf))); // wait event
  if(key_buf[4] == 'd') event->type = SDL_KEYDOWN; // key down xxx
  else event->type = SDL_KEYUP; // key up   xxx
  for(key_index = 0; key_buf[key_index + key_offset] != '\n'; key_index++)
    key_name[key_index] = key_buf[key_index + key_offset];
  key_name[key_index] = '\0';

  for(int i = 0; i < sizeof(keyname) / sizeof(char *); i++)
    if (strcmp(key_name, keyname[i]) == 0){
      event->key.keysym.sym = i;
      keystate[i] = ~event->type;
      ret = 1;
    }
  return ret;
}

int SDL_PeepEvents(SDL_Event *ev, int numevents, int action, uint32_t mask) {
  return 0;
}

uint8_t* SDL_GetKeyState(int *numkeys) {
  if(numkeys)
    *numkeys = sizeof(keystate) / sizeof(keystate[0]);
  return keystate;
}
