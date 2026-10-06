import json
from program_synthesis import IOExample, SynthesisTask, synthesize

def main():
    tasks = (
        SynthesisTask('add.v1', (IOExample((2,3),5), IOExample((-1,4),3), IOExample((0,7),7)), (IOExample((9,-2),7),)),
        SynthesisTask('composed.v1', (IOExample((3,1),8), IOExample((4,2),12), IOExample((5,3),16), IOExample((2,1),3)), (IOExample((6,1),35),)),
        SynthesisTask('contradictory.v1', (IOExample((2,1),1), IOExample((5,2),3)), (IOExample((1,5),-5),)),
        SynthesisTask('ambiguous.v1', (IOExample((0,0),0),), ()),
    )
    print(json.dumps({'grammar': 'finite-int-binary-v1', 'results': [synthesize(task).__dict__ for task in tasks]}, indent=2, sort_keys=True))

if __name__ == '__main__':
    main()
