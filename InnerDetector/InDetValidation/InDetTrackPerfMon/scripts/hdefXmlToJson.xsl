<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
    xmlns:xd="http://www.oxygenxml.com/ns/doc/xsl" exclude-result-prefixes="xd" version="1.0">
    <xd:doc scope="stylesheet">
        <xd:desc>
            <xd:p><xd:b>Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration</xd:b></xd:p>
            <xd:p><xd:b>Created on:</xd:b> Jun 26, 2024</xd:p>
            <xd:p><xd:b>Author:</xd:b>sroe</xd:p>
            <xd:p>An xslt transform to turn IDPVM xml histogram definition files to IDTPM json files.</xd:p>
            <xd:p>To run: <xd:pre>xsltproc -o output.json hdefXmlToJson.xsl hdef.xml</xd:pre></xd:p>
            <xd:p>Limitations: In its current form, does not produce the z axis nBins and limits</xd:p>
       </xd:desc>
    </xd:doc>
    <xsl:output indent="no" method="text"/>
    <xsl:strip-space elements="*"/>
    <!-- default to no output -->
    <xsl:template match="@* | node()"/>
    <xsl:template match="hdef"> { <xsl:apply-templates/>} </xsl:template>
    <xsl:template match="h[following-sibling::node()]"> "<xsl:value-of select="@id"/>" : { "type" : "<xsl:value-of
            select="@type"/>", <xsl:apply-templates/>},
    </xsl:template>
    <xsl:template match="h[not(following-sibling::node())]"> "<xsl:value-of select="@id"/>" : { "type" : "<xsl:value-of
            select="@type"/>", <xsl:apply-templates/>} </xsl:template>
    <xsl:template match="x"> "xAxis" : { "title" : "<xsl:value-of select="@title"/>", "nBins" :
            "<xsl:value-of select="@n"/>", "low" : "<xsl:value-of select="@lo"/>", "hi" :
            "<xsl:value-of select="@hi"/>"} </xsl:template>
    <xsl:template match="y[not(@n) and not(@lo)]">, "yAxis" : { "title" : "<xsl:value-of
            select="@title"/>"} </xsl:template>
    <xsl:template match="y[@n and not(@lo)]">, "yAxis" : { "title" : "<xsl:value-of select="@title"
        />", "nBins" : "<xsl:value-of select="@n"/>"} </xsl:template>
    <xsl:template match="y[@n and @lo]">, "yAxis" : { "title" : "<xsl:value-of select="@title"/>",
        "nBins" : "<xsl:value-of select="@n"/>", "low" : "<xsl:value-of select="@lo"/>", "high" :
            "<xsl:value-of select="@hi"/>" } </xsl:template>
</xsl:stylesheet>

