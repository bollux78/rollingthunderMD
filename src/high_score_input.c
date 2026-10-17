#include "rt_high_score_input.h"
void RT_initHighScoreInput(RT_HighScoreInput *state)
{
    if(!state)return;
    memset(state,0,sizeof(*state));
    memset(state->scan,1,sizeof state->scan);
    state->scan[18]=state->scan[26]=0;
    state->valid=TRUE;
}
bool RT_sampleHighScoreInput(RT_HighScoreInput *state,u16 pad1,u16 pad2)
{
    if(!state || !state->valid || state->fault)return FALSE;
    if(!RT_mapGenesisControlsToMcuScan(pad1,pad2,state->scan) ||
       !RT_serviceMcuInputScan(state->scan,state->records,state->combined,state->shared,&state->ready)){
        state->fault=TRUE;return FALSE;
    }
    for(u16 i=0;i<22;i+=2){
        state->pending[i]|=state->shared[48+i];
        state->pending[i+1]=state->shared[49+i];
    }
    return TRUE;
}
bool RT_takeHighScoreInput(RT_HighScoreInput *state,u8 input[22])
{
    if(!state || !state->valid || state->fault || !input)return FALSE;
    memcpy(input,state->pending,22);
    for(u16 i=0;i<22;i+=2)state->pending[i]=0;
    return TRUE;
}
