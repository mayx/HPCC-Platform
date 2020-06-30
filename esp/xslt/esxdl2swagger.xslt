<?xml version="1.0" encoding="UTF-8"?>
<!--
##############################################################################
#    HPCC SYSTEMS software Copyright (C) 2020 HPCC Systems®.
#
#    Licensed under the Apache License, Version 2.0 (the "License");
#    you may not use this file except in compliance with the License.
#    You may obtain a copy of the License at
#
#       http://www.apache.org/licenses/LICENSE-2.0
#
#    Unless required by applicable law or agreed to in writing, software
#    distributed under the License is distributed on an "AS IS" BASIS,
#    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#    See the License for the specific language governing permissions and
#    limitations under the License.
##############################################################################
-->

<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:xsd="http://www.w3.org/2001/XMLSchema" xmlns:soap="http://schemas.xmlsoap.org/wsdl/soap/" xmlns:http="http://schemas.xmlsoap.org/wsdl/http/" xmlns:mime="http://schemas.xmlsoap.org/wsdl/mime/" xmlns:wsdl="http://schemas.xmlsoap.org/wsdl/">
    <xsl:output method="text" omit-xml-declaration="yes" indent="no"/>
    <xsl:param name="version" select="/esxdl/@version"/>
    <xsl:param name="optionals" select="''"/>
    <xsl:param name="nocollapse" select="'0'"/>
    <xsl:param name="location" select="'http://localhost'"/>
    <xsl:template match="esxdl">
         <xsl:call-template name="CreateSwagger"/>
    </xsl:template>
    <xsl:template name="CreateSwagger">
        <xsl:text>openapi: 3.0.1
servers:
- url: </xsl:text><xsl:value-of select="$location"/>
        <xsl:apply-templates select="EsdlService"/>
        <xsl:apply-templates select="EsdlRequest|EsdlResponse"/>
        <xsl:apply-templates select="EsdlStruct"/>
        <xsl:apply-templates select="EsdlEnumType"/>
    </xsl:template>

    <xsl:template match="EsdlService">
        <xsl:text>
info:
  title: </xsl:text><xsl:value-of select="@name"/><xsl:text>
  version: '</xsl:text><xsl:value-of select="$version"/><xsl:text>'
paths:
</xsl:text>
        <xsl:variable name="servicename" select="@name"/>
        <xsl:for-each select="/esxdl/EsdlMethod">
        <xsl:text>  /</xsl:text><xsl:value-of select="$servicename"/><xsl:text>/</xsl:text><xsl:value-of select="@name"/>
        <xsl:text>:
    post:
      parameters:
        - name: ver_
          in: query
          schema:
            type: string
            default: </xsl:text><xsl:value-of select="$version"/>
        <xsl:if test="$optionals != ''">
            <xsl:call-template name="outputOptional">
                <xsl:with-param name="myoptionals" select="$optionals"/>
            </xsl:call-template>
        </xsl:if>
        <xsl:if test="@description"><xsl:text>
      description: </xsl:text><xsl:value-of select="@description"/>
      </xsl:if><xsl:text>
      requestBody:
        content:
          application/json:
            schema:
              type: object
              properties:
                </xsl:text><xsl:value-of select="@request_type"/><xsl:text>:
                  $ref: '#/components/schemas/</xsl:text><xsl:value-of select="@request_type"/><xsl:text>'
      responses:
        200:
          description: successful operation
          content:
            application/json:
              schema:
                type: object
                properties:
                  </xsl:text><xsl:value-of select="@response_type"/><xsl:text>:
                    $ref: '#/components/schemas/</xsl:text><xsl:value-of select="@response_type"/><xsl:text>'
        400:
          description: bad request
          content: {}
        401:
          description: unauthorized
          content: {}
</xsl:text>
        </xsl:for-each>
        <xsl:text>
components:
  schemas:</xsl:text>
    </xsl:template>

  <xsl:template match="EsdlRequest|EsdlResponse|EsdlStruct">
    <xsl:text>
    </xsl:text><xsl:value-of select="@name"/><xsl:text>:</xsl:text>
    <xsl:variable name="indent">
        <xsl:choose>
            <xsl:when test="@base_type and $nocollapse != '0'"><xsl:text>    </xsl:text></xsl:when>
            <xsl:otherwise></xsl:otherwise>
        </xsl:choose>
    </xsl:variable>
    <xsl:choose>
    <xsl:when test="@base_type and $nocollapse != '0'">
        <xsl:text>
      allOf:
        - $ref: '#/components/schemas/</xsl:text><xsl:value-of select="@base_type"/><xsl:text>'
        - type: object</xsl:text>
    </xsl:when>
    <xsl:otherwise>
        <xsl:text>
      type: object</xsl:text>
    </xsl:otherwise>
    </xsl:choose>
      <xsl:if test="*"><xsl:text>
      </xsl:text><xsl:value-of select="$indent"/><xsl:text>properties:</xsl:text>
        <xsl:for-each select="./*">
            <xsl:text>
        </xsl:text><xsl:value-of select="$indent"/><xsl:value-of select="@name"/><xsl:text>:</xsl:text>
        <xsl:if test="@required and @required != '0' and @required != 'false' and @required != 'no'"><xsl:text>
          </xsl:text><xsl:value-of select="$indent"/><xsl:text>required: true</xsl:text></xsl:if>
            <xsl:choose>
                <xsl:when test="name() = 'EsdlElement'">
                    <xsl:choose>
                        <xsl:when test="@complex_type">
                            <xsl:text>
          </xsl:text><xsl:value-of select="$indent"/><xsl:text>$ref: '#/components/schemas/</xsl:text><xsl:value-of select="@complex_type"/><xsl:text>'</xsl:text>
                        </xsl:when>
                        <xsl:otherwise>
                            <xsl:variable name="primitive">
                             <xsl:call-template name="outputPrimitive">
                                <xsl:with-param name="typename">
                                      <xsl:value-of select="@type"/>
                                </xsl:with-param>
                             </xsl:call-template>
                            </xsl:variable>
                            <xsl:variable name="format">
                             <xsl:call-template name="outputPrimFormat">
                                 <xsl:with-param name="typename">
                                      <xsl:value-of select="@type"/>
                                </xsl:with-param>
                             </xsl:call-template>
                            </xsl:variable>
                            <xsl:text>
          </xsl:text><xsl:value-of select="$indent"/><xsl:text>type: </xsl:text><xsl:value-of select="$primitive"/>
                            <xsl:if test="$format != ''">
                            <xsl:text>
          </xsl:text><xsl:value-of select="$indent"/><xsl:text>format: </xsl:text><xsl:value-of select="$format"/>
                            </xsl:if>
                        </xsl:otherwise>
                    </xsl:choose>
                </xsl:when>
                <xsl:when test="name() = 'EsdlArray'">
                    <xsl:text>
          </xsl:text><xsl:value-of select="$indent"/><xsl:text>type: object
          </xsl:text><xsl:value-of select="$indent"/><xsl:text>properties:</xsl:text>
                    <xsl:choose>
                        <xsl:when  test="@item_tag">
                            <xsl:text>
            </xsl:text><xsl:value-of select="$indent"/><xsl:value-of select="@item_tag"/><xsl:text>:</xsl:text>
                        </xsl:when>
                        <xsl:otherwise>
                            <xsl:text>
            </xsl:text><xsl:value-of select="$indent"/><xsl:text>item:</xsl:text>
                        </xsl:otherwise>
                    </xsl:choose>
                    <xsl:text>
              </xsl:text><xsl:value-of select="$indent"/><xsl:text>type: array
              </xsl:text><xsl:value-of select="$indent"/><xsl:text>items:</xsl:text>
                    <xsl:variable name="typename" select="@type"/>
                    <xsl:choose>
                        <xsl:when test="boolean(/esxdl/*/@name = $typename)">
                            <xsl:text>
                </xsl:text><xsl:value-of select="$indent"/><xsl:text>$ref: '#/components/schemas/</xsl:text><xsl:value-of select="@type"/><xsl:text>'</xsl:text>
                        </xsl:when>
                        <xsl:otherwise>
                            <xsl:variable name="primitive">
                             <xsl:call-template name="outputPrimitive">
                                <xsl:with-param name="typename">
                                      <xsl:value-of select="@type"/>
                                </xsl:with-param>
                             </xsl:call-template>
                            </xsl:variable>
                            <xsl:variable name="format">
                             <xsl:call-template name="outputPrimFormat">
                                <xsl:with-param name="typename">
                                      <xsl:value-of select="@type"/>
                                </xsl:with-param>
                             </xsl:call-template>
                            </xsl:variable>
                            <xsl:text>
                </xsl:text><xsl:value-of select="$indent"/><xsl:text>type: </xsl:text><xsl:value-of select="$primitive"/>
                            <xsl:if test="$format != ''">
                            <xsl:text>
                </xsl:text><xsl:value-of select="$indent"/><xsl:text>format: </xsl:text><xsl:value-of select="$format"/>
                            </xsl:if>
                        </xsl:otherwise>
                    </xsl:choose>
                </xsl:when>
                <xsl:when test="name() = 'EsdlEnum'">
                    <xsl:text>
          </xsl:text><xsl:value-of select="$indent"/><xsl:text>$ref: '#/components/schemas/</xsl:text><xsl:value-of select="@enum_type"/><xsl:text>'</xsl:text>
                </xsl:when>
            </xsl:choose>
        </xsl:for-each>
        </xsl:if>
  </xsl:template>

    <xsl:template match="EsdlEnumType">
      <xsl:if test="EsdlEnumItem">
          <xsl:variable name="primitive">
           <xsl:call-template name="outputPrimitive">
              <xsl:with-param name="typename">
                       <xsl:value-of select="@base_type"/>
              </xsl:with-param>
           </xsl:call-template>
         </xsl:variable>
         <xsl:variable name="format">
           <xsl:call-template name="outputPrimFormat">
              <xsl:with-param name="typename">
                       <xsl:value-of select="@base_type"/>
              </xsl:with-param>
           </xsl:call-template>
         </xsl:variable>
         <xsl:text>
    </xsl:text>
        <xsl:value-of select="@name"/><xsl:text>:
      type: </xsl:text><xsl:value-of select="$primitive"/>
        <xsl:if test="$format != ''">
           <xsl:text>
      format: </xsl:text><xsl:value-of select="$format"/>
        </xsl:if><xsl:text>
      enum:</xsl:text>
        <xsl:for-each select="EsdlEnumItem">
          <xsl:text>
        - </xsl:text><xsl:value-of select="@enum"/>
        </xsl:for-each>
      </xsl:if>
    </xsl:template>

    <xsl:template name="outputPrimitive">
        <xsl:param name="typename"/>
        <xsl:choose>
            <xsl:when test="$typename='bool'"><xsl:value-of select="'boolean'"/></xsl:when>
            <xsl:when test="$typename='boolean'"><xsl:value-of select="'boolean'"/></xsl:when>
            <xsl:when test="$typename='decimal'"><xsl:value-of select="'number'"/></xsl:when>
            <xsl:when test="$typename='float'"><xsl:value-of select="'number'"/></xsl:when>
            <xsl:when test="$typename='double'"><xsl:value-of select="'number'"/></xsl:when>
            <xsl:when test="$typename='integer'"><xsl:value-of select="'integer'"/></xsl:when>
            <xsl:when test="$typename='int64'"><xsl:value-of select="'integer'"/></xsl:when>
            <xsl:when test="$typename='long'"><xsl:value-of select="'integer'"/></xsl:when>
            <xsl:when test="$typename='int'"><xsl:value-of select="'integer'"/></xsl:when>
            <xsl:when test="$typename='short'"><xsl:value-of select="'integer'"/></xsl:when>
            <xsl:when test="$typename='nonPositiveInteger'"><xsl:value-of select="'integer'"/></xsl:when>
            <xsl:when test="$typename='negativeInteger'"><xsl:value-of select="'integer'"/></xsl:when>
            <xsl:when test="$typename='nonNegativeInteger'"><xsl:value-of select="'integer'"/></xsl:when>
            <xsl:when test="$typename='unsigned'"><xsl:value-of select="'integer'"/></xsl:when>
            <xsl:when test="$typename='unsignedLong'"><xsl:value-of select="'integer'"/></xsl:when>
            <xsl:when test="$typename='unsignedInt'"><xsl:value-of select="'integer'"/></xsl:when>
            <xsl:when test="$typename='unsignedShort'"><xsl:value-of select="'integer'"/></xsl:when>
            <xsl:when test="$typename='unsignedByte'"><xsl:value-of select="'integer'"/></xsl:when>
            <xsl:when test="$typename='positiveInteger'"><xsl:value-of select="'integer'"/></xsl:when>
            <xsl:when test="$typename='base64Binary'"><xsl:value-of select="'string'"/></xsl:when>
            <xsl:when test="$typename='string'"><xsl:value-of select="'string'"/></xsl:when>
            <xsl:when test="$typename='xsdString'"><xsl:value-of select="'string'"/></xsl:when>
            <xsl:when test="$typename='normalizedString'"><xsl:value-of select="'string'"/></xsl:when>
            <xsl:when test="$typename='binary'"><xsl:value-of select="'string'"/></xsl:when>
            <xsl:otherwise><xsl:value-of select="$typename"/></xsl:otherwise>
        </xsl:choose>
    </xsl:template>

    <xsl:template name="outputPrimFormat">
        <xsl:param name="typename"/>
        <xsl:choose>
            <xsl:when test="$typename='float'">float</xsl:when>
            <xsl:when test="$typename='double'">double</xsl:when>
            <xsl:when test="$typename='int64'">int64</xsl:when>
            <xsl:when test="$typename='long'">int64</xsl:when>
            <xsl:when test="$typename='unsignedLong'">int64</xsl:when>
            <xsl:when test="$typename='base64Binary'">byte</xsl:when>
            <xsl:when test="$typename='binary'">binary</xsl:when>
            <xsl:otherwise></xsl:otherwise>
        </xsl:choose>
    </xsl:template>

    <xsl:template name="outputOptional">
        <xsl:param name="myoptionals"/>
        <xsl:if test="string-length($myoptionals) > 0">
            <xsl:text>
        - name: </xsl:text><xsl:value-of select="substring-before(concat($myoptionals, ','), ',')"/><xsl:text>
          in: query
          schema:
            type: boolean
          allowEmptyValue: true</xsl:text>
            <xsl:call-template name="outputOptional">
                 <xsl:with-param name="myoptionals" select="substring-after($myoptionals, ',')"/>
            </xsl:call-template>
        </xsl:if>
    </xsl:template>

</xsl:stylesheet>
