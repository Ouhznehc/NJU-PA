#include <NDL.h>
#include <SDL.h>
#include <string.h>
#include <assert.h>

enum{PollEvent, WaitEvent};

#define keyname(k) #k,
#define key_offset 9
static const char *keyname[] = {
  "NONE",
  _KEYS(keyname)
};
static uint8_t key_state[sizeof(keyname) / sizeof(keyname[0])] = {0};
int SDL_PushEvent(SDL_Event *ev) {
  assert(0);
  return 0;
}

int SDL_DealEvent(SDL_Event *event, int select) {
 char key_buf[64], key_name[64];
  int key_index = 0;
  if(select == PollEvent)
    if(!NDL_PollEvent(key_buf, sizeof(key_buf))) return 0; // return intsantly
  if(select == WaitEvent)
    while(!NDL_PollEvent(key_buf, sizeof(key_buf))); // wait event
  if(key_buf[4] == 'u')event->type = SDL_KEYUP; // key up   xxx
  else event->type = SDL_KEYDOWN; // key down xxx
  for(key_index = 0; key_buf[key_index + key_offset] != '\n'; key_index++)
    key_name[key_index] = key_buf[key_index + key_offset];
  key_name[key_index] = '\0';

  for(int i = 0; i < sizeof(keyname) / sizeof(char *); i++)
    if (strcmp(key_name, keyname[i]) == 0){
      event->key.keysym.sym = i;
      key_state[i] = !event->type;
    }
  return 1;
}

int SDL_PollEvent(SDL_Event *event) {
  return SDL_DealEvent(event, PollEvent);
}

int SDL_WaitEvent(SDL_Event *event) {
  return SDL_DealEvent(event, WaitEvent);
}

int SDL_PeepEvents(SDL_Event *ev, int numevents, int action, uint32_t mask) {
  assert(0);
  return 0;
}

uint8_t* SDL_GetKeyState(int *numkeys) {
  if (numkeys) *numkeys = sizeof(key_state) / sizeof(uint8_t);
  return key_state;
}
