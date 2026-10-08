"""Fixed-protocol LLM reference on the independent MInDS-14 holdout.

This is a zero-shot LLM reference, not a fine-tuned transformer. It is kept
separate from supervised baselines and never contributes labels to HERUS.
"""
from __future__ import annotations
import concurrent.futures as cf
import json, time
from pathlib import Path
from openai import OpenAI
from independent_minds14_benchmark import fetch_rows, split
from real_data_baseline_benchmark import metrics

MODEL='gpt-5.5'; WORKERS=4
SCHEMA={'type':'json_schema','json_schema':{'name':'intent_prediction','strict':True,'schema':{'type':'object','properties':{'label':{'type':'string'}},'required':['label'],'additionalProperties':False}}}

def classify(client, text, labels):
    prompt=('Classifique a mensagem em exatamente uma classe da lista. Retorne apenas JSON. '
            'Não explique. Classes: '+', '.join(labels)+'\nMensagem: '+text)
    for attempt in range(4):
        try:
            r=client.chat.completions.create(model=MODEL,messages=[
                {'role':'system','content':'Você é um classificador determinístico de intenções. Nunca invente uma classe.'},
                {'role':'user','content':prompt}],response_format=SCHEMA,max_completion_tokens=500,extra_body={'reasoning':{'effort':'none'}})
            if not r.choices[0].message.content:
                raise RuntimeError(f"empty_llm_content:{r.choices[0].finish_reason}")
            label=json.loads(r.choices[0].message.content)['label']
            return label, {'prompt_tokens':r.usage.prompt_tokens,'completion_tokens':r.usage.completion_tokens}
        except Exception:
            if attempt==3: raise
            time.sleep(2**attempt)

def run():
    rows=fetch_rows(); _,_,hold=split(rows); labels=sorted({r['label'] for r in rows}); client=OpenAI()
    def one(row):
        label,usage=classify(client,row['text'],labels)
        return {'example_id':row['path'],'label':row['label'],'prediction':label,'correct':label==row['label'],'usage':usage}
    with cf.ThreadPoolExecutor(max_workers=WORKERS) as pool:
        ledger=list(pool.map(one,hold))
    predictions=[x['prediction'] for x in ledger]
    out={'schema':'herus-llm-reference-minds14-v1','model':MODEL,'dataset':{'id':'PolyAI/minds14','config':'pt-PT','holdout':len(hold),'labels':len(labels)},'protocol':{'mode':'zero-shot','fixed_prompt':True,'fit_labels_used_only_for_class_list':True,'training_on_dataset':False,'claim_boundary':'LLM reference only; not a transformer-equivalent or SOTA claim'},'metrics':metrics(hold,predictions),'ledger':ledger,'usage':{'prompt_tokens':sum(x['usage']['prompt_tokens'] for x in ledger),'completion_tokens':sum(x['usage']['completion_tokens'] for x in ledger)}}
    p=Path('research/evidence/llm_minds14_reference_v1.json'); p.write_text(json.dumps(out,indent=2,sort_keys=True,ensure_ascii=False)+'\n'); return out

if __name__=='__main__':
    out=run(); print(json.dumps({'path':'research/evidence/llm_minds14_reference_v1.json','model':MODEL,'metrics':out['metrics'],'usage':out['usage']},ensure_ascii=False,indent=2))
