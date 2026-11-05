"""Read-only reconstruction of the RNG sequence encoded by saved coefficients.

The actual Direct3D8 device selects x87 PC24. Each elementary operation below
rounds to binary32, matching the separately instruction-verified arithmetic.
The highest RNG bit is deliberately left ambiguous: rand() observes15 bits.
"""
import struct

def f32(value):return struct.unpack('<f',struct.pack('<f',value))[0]
def bits(value):return struct.unpack('<I',struct.pack('<f',value))[0]
def coefficient(radius,sample):
    negative=-radius
    return f32(f32(f32(f32(radius-negative)*sample)*f32(3.0518509447574615e-05))+negative)

def audit_coefficients(words,events):
    assert len(words)==len(events)==383
    radii=[f32(float(event.find('Data').get('Range','0'))) for event in events]
    tables={radius:{bits(coefficient(radius,n)):n for n in range(32768)} for radius in set(radii) if radius!=0}
    assert all(len(table)==32768 for table in tables.values())
    observations=[]
    for index,(word,radius) in enumerate(zip(words,radii)):
        if radius==0:assert word==0,(index,word)
        else:
            assert word in tables[radius],(index,word,radius)
            observations.append((index,tables[radius][word]))
    assert len(observations)>=3
    first,n=observations[0];mask=0x7fffffff;multiplier=0x343fd;increment=0x269ec3
    solutions=[]
    for low in range(65536):
        state=(n<<16)|low;previous=first
        for index,expected in observations[1:]:
            for _ in range(index-previous):state=(state*multiplier+increment)&mask
            previous=index
            if state>>16!=expected:break
        else:solutions.append((n<<16)|low)
    assert len(solutions)==1,solutions
    inverse=pow(multiplier,-1,1<<31);start=solutions[0]
    for _ in range(first+1):start=((start-increment)*inverse)&mask
    state=start;recreated=[]
    for radius in radii:
        state=(state*multiplier+increment)&mask
        recreated.append(bits(coefficient(radius,state>>16)))
    assert recreated==words
    return dict(verified_complete_coefficient_words=len(words),nonzero_range_observations=len(observations),
        recovered_rng_start_low31=start,recovered_rng_end_low31=state,
        highest_rng_bit_unobservable=True,underlying_rng_state_claimed_captured=False,
        arithmetic_precision='PC24',rand_reference_address='00581021',randomization_reference_address='004ae570',
        original_memory_or_profile_written=False)
