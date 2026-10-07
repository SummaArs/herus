"""Aggregate already-run real ML evidence into a fair competition scorecard."""
from __future__ import annotations
import json
from pathlib import Path

ROOT=Path(__file__).parent
E=ROOT/'evidence'

def load(name): return json.loads((E/name).read_text())

def row(dataset, model, metrics, family, source):
    return {'dataset':dataset,'model':model,'family':family,'source':source,**{k:metrics.get(k) for k in ('accuracy','macro_f1','coverage','selective_accuracy','infer_ms','train_ms')}}

def build():
    rows=[]
    m=load('real_data_baselines_mintrec_v2.json')['methods']
    for name,metrics in m.items(): rows.append(row('MIntRec S06',name,metrics,'supervised / HERUS adapter','real_data_baselines_mintrec_v2.json'))
    for name,metrics in load('ml_paradigms_mintrec_v2.json')['methods'].items(): rows.append(row('MIntRec S06',name,metrics,'unsupervised / self-supervised / RL proxy','ml_paradigms_mintrec_v2.json'))
    for item in load('tensor_network_mintrec_v1.json')['ranks']:
        rows.append(row('MIntRec S06',f"tensor_train_rank_{item['rank']}",item,'tensor network','tensor_network_mintrec_v1.json'))
    t=load('transformer_mintrec_v1.json')['holdout_metrics']; rows.append(row('MIntRec S06','bert_tiny',t,'transformer','transformer_mintrec_v1.json'))
    s=load('selective_margin_calibration_mintrec_v1.json')['holdout']['0.95']; rows.append(row('MIntRec S06','herus_selective_target_95',s,'HERUS selective assurance','selective_margin_calibration_mintrec_v1.json'))
    for item in load('ml_reference_matrix_minds14_v1.json')['models']:
        rows.append(row('MInDS-14 pt-PT',item['model'],item,'classical supervised','ml_reference_matrix_minds14_v1.json'))
    return rows

def main():
    rows=build()
    by_dataset={}
    for r in rows: by_dataset.setdefault(r['dataset'],[]).append(r)
    ranking={}
    for dataset,items in by_dataset.items():
        ranking[dataset]=sorted(items,key=lambda r:(r['accuracy'] or -1,r['macro_f1'] or -1),reverse=True)
    pareto=[]
    for r in rows:
        dominated=False
        for q in rows:
            if q is r or q['dataset']!=r['dataset']: continue
            if (q['selective_accuracy'] or 0)>=(r['selective_accuracy'] or 0) and (q['coverage'] or 0)>=(r['coverage'] or 0) and ((q['selective_accuracy'] or 0)>(r['selective_accuracy'] or 0) or (q['coverage'] or 0)>(r['coverage'] or 0)):
                dominated=True; break
        if not dominated: pareto.append(r)
    evidence={'protocol':'aggregate-only; no refit; no holdout touched','rows':rows,'ranking':ranking,'pareto':pareto,'claims':{'universal_winner':False,'herus_current_status':'wins selective risk tradeoff only; does not win full-coverage accuracy','next_gate':'independent datasets and repeated seeds'}}
    (E/'final_ml_competition_v1.json').write_text(json.dumps(evidence,indent=2,ensure_ascii=False)+'\n')
    lines=['# Competição final de ML — scorecard real','', '> Este relatório agrega experimentos já executados. Não treina novamente nem transforma um dataset em prova universal.','', '## Resultado honesto','', 'O HERUS **ainda não vence todos os algoritmos**. No MIntRec, o melhor baseline de cobertura total é o Naive Bayes (49,22% de acurácia) e o Tensor-Train rank 64 chega a 42,23%. O HERUS seletivo atinge 93,18% de precisão, mas aceita apenas 22,80% dos casos. São objetivos diferentes.','', '| Dataset | Método | Família | Acurácia | Macro-F1 | Cobertura | Precisão seletiva |', '|---|---|---|---:|---:|---:|---:|']
    for r in sorted(rows,key=lambda x:(x['dataset'],-(x['accuracy'] or 0))): lines.append(f"| {r['dataset']} | {r['model']} | {r['family']} | {r['accuracy'] if r['accuracy'] is not None else '-'} | {r['macro_f1'] if r['macro_f1'] is not None else '-'} | {r['coverage'] if r['coverage'] is not None else '-'} | {r['selective_accuracy'] if r['selective_accuracy'] is not None else '-'} |")
    lines += ['', '## Interpretação', '', '- **Cobertura total:** HERUS ainda perde para baselines supervisionados no MIntRec.', '- **Assurance seletiva:** HERUS reduz risco ao abster-se; isso não é vitória de classificação geral.', '- **Transformer:** o checkpoint pequeno testado ficou abaixo dos baselines clássicos; isso não representa todos os transformers.', '- **Reforço:** o resultado é apenas proxy contextual; não houve ambiente com transições reais.', '- **MInDS-14:** SVM linear e floresta atingiram 95,92%; HERUS não foi adaptado a esse corpus nesta rodada.', '', '## Próximo critério de 100%', '', 'Só considerar avanço para 100% após: múltiplos datasets independentes, seeds repetidas, intervalos de confiança, teste estatístico pareado, custo medido e comparação com modelos fortes ajustados de forma justa.']
    Path('docs/120-COMPETICAO-FINAL-ML-SCORECARD.md').write_text('\n'.join(lines)+'\n')
    print(json.dumps({'datasets':list(by_dataset),'models':len(rows),'pareto_points':len(pareto),'universal_winner':False},indent=2))
if __name__=='__main__': main()
