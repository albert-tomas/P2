#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include "pav_analysis.h"

#include "vad.h"

const float FRAME_TIME = 10.0F; /* in ms. */

/*
 * As the output state is only ST_VOICE, ST_SILENCE, or ST_UNDEF,
 * only this labels are needed. You need to add all labels, in case
 * you want to print the internal state in string format
 */

const char *state_str[] = {
    "UNDEF", "S", "V", "INIT"};

const char *state2str(VAD_STATE st)
{
  return state_str[st];
}

/* Define a datatype with interesting features */
typedef struct
{
  float zcr;
  float p;
  float am;
} Features;

/*
 * TODO: Delete and use your own features!
 */

Features compute_features(const float *x, int N)
{
  /*
   * Input: x[i] : i=0 .... N-1
   * Ouput: computed features
   */
  /*
   * DELETE and include a call to your own functions
   *
   * For the moment, compute random value between 0 and 1
   */
  Features feat;
  feat.zcr = compute_zcr(x, N, 16000);
  feat.p = compute_power(x, N); //.p = potencia media de trama
  feat.am = compute_am(x, N);   //.am = amplitud media
  return feat;
}

/*
 * TODO: Init the values of vad_data
 */

VAD_DATA *vad_open(float rate, float alfa1, float alfa2, unsigned int N_tramas_ini)
{
  VAD_DATA *vad_data = malloc(sizeof(VAD_DATA));
  vad_data->state = ST_INIT;
  vad_data->sampling_rate = rate;
  vad_data->frame_length = rate * FRAME_TIME * 1e-3;

  vad_data->N_ini = 0;
  vad_data->N_tramas_ini = N_tramas_ini;

  vad_data->k0 = 0;
  vad_data->k1 = 0;

  vad_data->alfa1 = alfa1;
  vad_data->alfa2 = alfa2;

  vad_data->num_MS = 0; // Numero de tramas en estado MAYBE_SILENCE
  vad_data->num_MV = 0; // Numero de tramas en estado MAYBE_VOICE

  vad_data->max_MS = 9;  // Número máximo de frames en qué decidimos si pasamos de V a S
  vad_data->max_MV = 67; // Número máximo de frames en qué decidimos si pasamos de S a V
  vad_data->min_S = 6;   // Duración mínima de SILENCE
  vad_data->min_V = 0;   // Duración mínima de VOICE

  return vad_data;
}

VAD_STATE vad_close(VAD_DATA *vad_data)
{
  /*
   * TODO: decide what to do with the last undecided frames
   */
  VAD_STATE state = vad_data->state;

  if(vad_data->state == ST_SILENCE || vad_data->state == ST_VOICE){
    state = vad_data->state;
  } else {
    state = ST_SILENCE;
  }

  free(vad_data);
  return state;
}

unsigned int vad_frame_size(VAD_DATA *vad_data)
{
  return vad_data->frame_length;
}

/*
 * TODO: Implement the Voice Activity Detection
 * using a Finite State Automata
 */

VAD_STATE vad(VAD_DATA *vad_data, float *x)
{

  /*
   * TODO: You can change this, using your own features,
   * program finite state automaton, define conditions, etc.
   */

  Features f = compute_features(x, vad_data->frame_length);
  vad_data->last_feature = f.p; /* save feature, in case you want to show */

  switch (vad_data->state)
  {
  case ST_INIT:

    // Contamos las primeras N tramas
    vad_data->N_ini++;

    vad_data->k0 = vad_data->k0 + pow(10, (f.p / 10));

    if (vad_data->N_ini == vad_data->N_tramas_ini)
    {
      vad_data->k0 = 10 * log10(vad_data->k0 / vad_data->N_ini); // Calculada como el valor en decibelios de la potencia media durante las Ninit primeras tramas
      vad_data->k1 = vad_data->k0 + vad_data->alfa1;             // Límite inferior
      vad_data->k2 = vad_data->k1 + vad_data->alfa2;             // Límite superior

      vad_data->state = ST_SILENCE;
    }

    break;

  case ST_SILENCE:
    if (f.p > vad_data->k1)
      vad_data->state = ST_MAYBE_VOICE;
    break;

  case ST_VOICE:
    if (f.p < vad_data->k2)
      vad_data->state = ST_MAYBE_SILENCE;
    break;

  case ST_MAYBE_VOICE:
    if (vad_data->num_MV >= vad_data->max_MV)
    {
      vad_data->num_MV = 0;
      vad_data->state = ST_SILENCE;
    }
    else if ((f.p > vad_data->k2) && (vad_data->num_MV >= vad_data->min_V))
    {
      vad_data->num_MV = 0;
      vad_data->state = ST_VOICE;
    }
    else
      vad_data->num_MV++;
    break;

  case ST_MAYBE_SILENCE:
    if (((f.p < vad_data->k1) && vad_data->num_MS >= vad_data->min_S) || vad_data->num_MS >= vad_data->max_MS)
    {
      vad_data->num_MS = 0;
      vad_data->state = ST_SILENCE;
    }
    else if (f.p > vad_data->k2)
    {
      vad_data->num_MS = 0;
      vad_data->state = ST_VOICE;
    }
    else
    {
      vad_data->num_MS++;
    }
    break;

  case ST_UNDEF:
    break;
  }

  /*if(vad_data->state == ST_SILENCE)
   printf("SILENCE\n");
  if(vad_data->state == ST_VOICE)
   printf("VOICE\n");
  if(vad_data->state == ST_MAYBE_SILENCE)
   printf("MAYBE_SILENCE\n");
  if(vad_data->state == ST_MAYBE_VOICE)
   printf("MAYBE_VOICE\n");
  if(vad_data->state == ST_UNDEF)
   printf("UNDEF\n");*/

  if (vad_data->state == ST_SILENCE || vad_data->state == ST_VOICE)
  {
    return vad_data->state;
  }
  else if (vad_data->state == ST_INIT)
  {
    return ST_SILENCE;
  }
  else
  {
    return ST_UNDEF;
  }
}

void vad_show_state(const VAD_DATA *vad_data, FILE *out)
{
  fprintf(out, "%d\t%f\n", vad_data->state, vad_data->last_feature);
}