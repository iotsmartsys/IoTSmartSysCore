# Claude Code — adaptador para o EKOM

Este repositório usa [`AGENTS.md`](AGENTS.md) como fonte obrigatória de
instruções para agentes. Este arquivo apenas garante o roteamento no Claude
Code; ele não substitui nem resume o EKOM.

Antes de investigar, editar arquivos ou executar validações:

1. leia `AGENTS.md` integralmente;
2. confirme que a ordem identifica capacidade, resultado, recorte e especificação,
   quando aplicável;
3. use a tabela de roteamento de `AGENTS.md` para ler integralmente
   `REGRAS-COMUNS.md` e exatamente o perfil da capacidade recebida;
4. cumpra as condições de entrada antes de iniciar a atuação.

As regras carregadas por esse roteamento são mandatórias. Em particular:

- não altere implementação quando a ordem e a capacidade não autorizarem;
- atualize a especificação e o conhecimento EKOM exigidos pelo perfil;
- não trate tarefa, comando, build ou teste pendente como concluído;
- editar arquivos não encerra a etapa;
- quando o perfil exigir entrega Git, a etapa termina somente após resultado
  material, commit, push e árvore de trabalho limpa.

Se qualquer fonte obrigatória estiver inacessível ou uma condição de entrada
falhar, pare e informe o impedimento. Não improvise outro papel ou contrato.
