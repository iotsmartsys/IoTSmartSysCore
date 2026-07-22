# Especificação — Release e Distribuição

**ID:** IOTSSC-RELEASE

**Estado normativo:** Active

**Estado de implementação:** In Progress

**Última atualização:** 22/07/2026

## 1. Objetivo

Definir um release reprodutível, rastreável e seguro da biblioteca para o PlatformIO.

## 2. Requisitos

- **REL-001:** releases oficiais só podem ser criados a partir da branch `main`; a automação deve falhar fora dela.
- **REL-002:** a versão deve seguir o versionamento adotado pelo projeto e permanecer consistente entre `library.json`, header público e tag.
- **REL-003:** deve existir uma única localização canônica para o header gerado de versão: `src/Version/IoTSmartSysCoreVersion.h`.
- **REL-004:** a automação local deve atualizar a versão, atualizar o header, criar o commit de release e criar a tag correspondente.
- **REL-005:** push de commit e tag é operação externa e requer intenção explícita do operador.
- **REL-006:** a tag na `main` deve acionar o GitHub Actions responsável pelo empacotamento e publicação no PlatformIO.
- **REL-007:** o pacote publicado deve ser construído a partir do mesmo commit identificado pela tag.
- **REL-008:** falhas de versão, branch, build, empacotamento ou publicação devem interromper o release sem declarar sucesso parcial como release concluído.

## 3. Fluxo esperado

```text
main limpa e atualizada
→ escolher incremento de versão
→ atualizar library.json e header canônico
→ validar
→ commit de release
→ tag anotada
→ push autorizado
→ pipeline empacota e publica no PlatformIO
```

## 4. Desvios conhecidos

- **REL-DEV-001:** o alvo atual do `Makefile` não impede release fora da `main`.
- **REL-DEV-002:** o workflow e o `Makefile` referenciam localizações diferentes para o header de versão.

Esses desvios mantêm o estado de implementação como `In Progress` e estão registrados em `EKM-GAP-0002`.

## 5. Fora de escopo

- alterar o provedor de pacotes;
- trocar GitHub Actions;
- executar release nesta transação;
- implementar agora as correções dos desvios.

## 6. Critérios de aceite

- tentativa fora de `main` falha antes de modificar versão;
- versão consistente nos três identificadores;
- build e empacotamento aprovados;
- tag aponta para o commit publicado;
- pipeline publica somente após todas as validações obrigatórias;
- execução de teste não cria push ou publicação real sem autorização.

## 7. Relações

- `docs/rfc/KNOWLEDGE-MAP.md`;
- `EKM-GAP-0002`.
