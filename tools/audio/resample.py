import numpy as np
def resample(samples,rate,source_rate=3579545/64,start_sample=0):
    # Anti-aliasing low-pass Lanczos sinc at the exact rational chip sample rate.
    count=round((len(samples)-start_sample)*rate/source_rate)
    ratio=min(1.,rate/source_rate);radius=32/ratio
    output=np.zeros(count)
    for start in range(0,count,4096):
        positions=start_sample+np.arange(start,min(start+4096,count))*source_rate/rate
        offsets=np.arange(-int(np.ceil(radius)),int(np.ceil(radius))+1)
        indices=np.floor(positions).astype(int)[:,None]+offsets
        distance=positions[:,None]-indices
        weights=ratio*np.sinc(distance*ratio)*np.sinc(distance/radius)
        weights[np.abs(distance)>=radius]=0
        valid=(indices>=0)&(indices<len(samples))
        data=samples[np.clip(indices,0,len(samples)-1)]*valid
        output[start:start+len(positions)]=(data*weights).sum(axis=1)/weights.sum(axis=1)
    return output

