# -*- coding: utf-8 -*-
"""Gera relatorio.docx a partir de relatorio.md.

O template ABNT da UVA e usado como --reference-doc, mas ele nao define varios
estilos que o pandoc emite (FirstParagraph, Compact, VerbatimChar, Table,
SectionNumber) e define Corpo de texto em negrito. Sem os ajustes abaixo o
documento sai com tabelas sem borda, codigo em Times e paragrafos em negrito.

Uso:  python gerar-relatorio.py
"""
import io
import os
import re
import shutil
import subprocess
import sys
import zipfile

AQUI = os.path.dirname(os.path.abspath(__file__))
MD = os.path.join(AQUI, "relatorio.md")
DOCX = os.path.join(AQUI, "relatorio.docx")
TEMPLATE = os.path.join(
    AQUI, "..", "..", "..", "..", "4º Período",
    "Laboratório de Desenvolvimento de Software - Sábado",
    "PESQUISA_CMP- ECP - SIF_TEMPLATE Novo4.docx")

TITULO = "Analisador lexico da linguagem Domus"
AUTOR = "Caio Parada Oliveira Planinschek"

# Estilos que o pandoc referencia e o template nao define.
ESTILOS_NOVOS = """
<w:style w:type="paragraph" w:customStyle="1" w:styleId="FirstParagraph">
  <w:name w:val="First Paragraph"/><w:basedOn w:val="Normal"/><w:qFormat/>
</w:style>
<w:style w:type="paragraph" w:customStyle="1" w:styleId="Compact">
  <w:name w:val="Compact"/><w:basedOn w:val="Normal"/><w:qFormat/>
  <w:pPr><w:spacing w:line="240" w:lineRule="auto"/><w:ind w:firstLine="0"/>
  <w:contextualSpacing/><w:jc w:val="left"/></w:pPr>
</w:style>
<w:style w:type="character" w:customStyle="1" w:styleId="VerbatimChar">
  <w:name w:val="Verbatim Char"/><w:basedOn w:val="Fontepargpadro"/>
  <w:rPr><w:rFonts w:ascii="Courier New" w:hAnsi="Courier New" w:cs="Courier New"/>
  <w:sz w:val="20"/><w:szCs w:val="20"/></w:rPr>
</w:style>
<w:style w:type="character" w:customStyle="1" w:styleId="SectionNumber">
  <w:name w:val="Section Number"/><w:basedOn w:val="Fontepargpadro"/>
</w:style>
<w:style w:type="table" w:customStyle="1" w:styleId="Table">
  <w:name w:val="Table"/><w:basedOn w:val="Tabelacomgrade"/><w:uiPriority w:val="59"/>
</w:style>
"""

# Corpo de texto do template e negrito, sem justificacao e sem entrelinha 1,5.
# O pandoc usa esse estilo em todo paragrafo que nao abre uma secao.
CORPO_NOVO = ('<w:style w:styleId="Corpodetexto" w:type="paragraph">'
              '<w:name w:val="Body Text"/><w:basedOn w:val="Normal"/>'
              '<w:link w:val="CorpodetextoChar"/></w:style>')

# Bloco de codigo herdava justificacao, recuo de primeira linha e entrelinha 1,5.
FONTE_NOVA = ('<w:style w:type="paragraph" w:customStyle="1" w:styleId="SourceCode">'
              '<w:name w:val="Source Code"/><w:basedOn w:val="Normal"/>'
              '<w:link w:val="VerbatimChar"/>'
              '<w:pPr><w:wordWrap w:val="off"/>'
              '<w:spacing w:line="240" w:lineRule="auto"/>'
              '<w:ind w:firstLine="0"/><w:jc w:val="left"/></w:pPr>'
              '<w:rPr><w:rFonts w:ascii="Courier New" w:hAnsi="Courier New" '
              'w:cs="Courier New"/><w:sz w:val="20"/><w:szCs w:val="20"/></w:rPr>'
              '</w:style>')

CORE = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>\n'
        '<cp:coreProperties '
        'xmlns:cp="http://schemas.openxmlformats.org/package/2006/metadata/'
        'core-properties" xmlns:dc="http://purl.org/dc/elements/1.1/" '
        'xmlns:dcterms="http://purl.org/dc/terms/" '
        'xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance">'
        '<dc:title>%s</dc:title><dc:creator>%s</dc:creator>'
        '<cp:lastModifiedBy>%s</cp:lastModifiedBy><cp:keywords></cp:keywords>'
        '<dc:description></dc:description></cp:coreProperties>' % (TITULO, AUTOR, AUTOR))

APP = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>\n'
       '<Properties xmlns="http://schemas.openxmlformats.org/officeDocument/2006/'
       'extended-properties"><Company></Company><Manager></Manager>'
       '<Application>Microsoft Office Word</Application>'
       '<DocSecurity>0</DocSecurity></Properties>')

CUSTOM = ('<?xml version="1.0" encoding="UTF-8"?>'
          '<Properties xmlns="http://schemas.openxmlformats.org/officeDocument/2006/'
          'custom-properties" xmlns:vt="http://schemas.openxmlformats.org/'
          'officeDocument/2006/docPropsVTypes" />')


def rodar_pandoc():
    cmd = ["pandoc", MD, "-o", DOCX, "--number-sections",
           "--reference-doc=" + os.path.normpath(TEMPLATE)]
    subprocess.check_call(cmd)


def ajustar_estilos(xml):
    # Corpo de texto: sem negrito, herdando Normal.
    novo, n = re.subn(r'<w:style w:styleId="Corpodetexto" w:type="paragraph">.*?</w:style>',
                      lambda m: CORPO_NOVO, xml, flags=re.S)
    assert n == 1, "estilo Corpodetexto nao encontrado"
    xml = novo
    # Bloco de codigo em monoespacada, alinhado a esquerda.
    novo, n = re.subn(r'<w:style w:type="paragraph" w:customStyle="1" '
                      r'w:styleId="SourceCode">.*?</w:style>',
                      lambda m: FONTE_NOVA, xml, flags=re.S)
    assert n == 1, "estilo SourceCode nao encontrado"
    xml = novo
    # Corpo do texto em 12 pt, como pede a ABNT (o template traz 11).
    novo, n = re.subn(r'(<w:style w:default="1" w:styleId="Normal" w:type="paragraph">'
                      r'.*?)<w:sz w:val="22" />',
                      lambda m: m.group(1) + '<w:sz w:val="24" />', xml, flags=re.S)
    assert n == 1, "tamanho do estilo Normal nao encontrado"
    xml = novo
    # Os estilos de titulo do template trazem numeracao automatica de lista, que
    # somaria ao numero que o pandoc ja escreve ("1 1 INTRODUCAO"). Fica so o
    # numero do pandoc, que e o que alimenta tambem o sumario.
    n = 0
    for nivel in range(1, 10):
        xml, k = re.subn(r'(<w:style w:styleId="Ttulo%d" w:type="paragraph">.*?)'
                         r'<w:numPr>.*?</w:numPr>' % nivel,
                         lambda m: m.group(1), xml, flags=re.S)
        n += k
    assert n > 0, "numeracao automatica dos titulos nao encontrada"

    # O sumario do template reserva 6 pt antes e depois de cada entrada, o que
    # faz as 33 entradas ocuparem uma pagina inteira sem folga.
    for nivel in range(1, 4):
        xml = re.sub(r'(<w:style w:styleId="Sumrio%d" w:type="paragraph">.*?)'
                     r'<w:spacing w:after="120" w:before="120" />' % nivel,
                     lambda m: m.group(1) + '<w:spacing w:after="0" w:before="0" />',
                     xml, flags=re.S)

    # Estilos ausentes no template.
    assert xml.count("</w:styles>") == 1
    return xml.replace("</w:styles>", ESTILOS_NOVOS + "</w:styles>")


def ajustar_documento(xml, id_cabecalho):
    # O pandoc separa o numero da secao do titulo com uma tabulacao; em paragrafo
    # centralizado ela abre um vao. Uma tabulacao vira um espaco.
    def troca(m):
        return re.sub(r'<w:r><w:tab ?/></w:r>',
                      '<w:r><w:t xml:space="preserve"> </w:t></w:r>', m.group(0))

    xml, n = re.subn(r'<w:p><w:pPr><w:pStyle w:val="Ttulo[1-9]" ?/>.*?</w:p>',
                     troca, xml, flags=re.S)
    assert n > 0, "nenhum titulo encontrado"
    # A ultima secao (parte textual) usa o cabecalho com o numero de pagina.
    corte = xml.rfind("<w:sectPr")
    cauda = xml[corte:]
    cauda, n = re.subn(r'<w:headerReference r:id="[^"]+" w:type="default"',
                       '<w:headerReference r:id="%s" w:type="default"' % id_cabecalho,
                       cauda)
    assert n == 1, "cabecalho padrao da ultima secao nao encontrado"
    return xml[:corte] + cauda


def cabecalho_com_numero(z):
    """Devolve o rId do cabecalho que contem o campo PAGE."""
    rels = z.read("word/_rels/document.xml.rels").decode("utf-8")
    achados = [(alvo, rid)
               for rid, alvo in re.findall(r'Id="([^"]+)"[^>]*Target="([^"]+)"', rels)
               if alvo.startswith("header")
               and "PAGE" in z.read("word/" + alvo).decode("utf-8")]
    if not achados:
        raise SystemExit("nenhum cabecalho com numero de pagina no template")
    return sorted(achados)[0][1]


def ajustar_docx():
    tmp = DOCX + ".tmp"
    with zipfile.ZipFile(DOCX) as z:
        rid = cabecalho_com_numero(z)
        itens = [(i, z.read(i.filename)) for i in z.infolist()]
    with zipfile.ZipFile(tmp, "w", zipfile.ZIP_DEFLATED) as saida:
        for info, dados in itens:
            nome = info.filename
            if nome == "word/styles.xml":
                dados = ajustar_estilos(dados.decode("utf-8")).encode("utf-8")
            elif nome == "word/document.xml":
                dados = ajustar_documento(dados.decode("utf-8"), rid).encode("utf-8")
            elif nome == "docProps/core.xml":
                dados = CORE.encode("utf-8")
            elif nome == "docProps/app.xml":
                dados = APP.encode("utf-8")
            elif nome == "docProps/custom.xml":
                dados = CUSTOM.encode("utf-8")
            saida.writestr(info, dados)
    shutil.move(tmp, DOCX)
    return rid


def main():
    rodar_pandoc()
    rid = ajustar_docx()
    print("relatorio.docx gerado (cabecalho numerado: %s)." % rid)
    print("Abra no Word e atualize o sumario com Ctrl+A e F9 antes de exportar o PDF.")


if __name__ == "__main__":
    sys.exit(main())
