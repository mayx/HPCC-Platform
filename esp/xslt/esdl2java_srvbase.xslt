<?xml version="1.0" encoding="UTF-8"?>
<!--
##############################################################################
#    HPCC SYSTEMS software Copyright (C) 2015 HPCC Systems.
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

<!--
TODO:
- Check defualts are handled in unserializing (done by original code)
- Test different primitive types for unserialze and serialize, especially serialize
- binary, base64binary, unsigned byte
- Better exception handling
- Compare results with original implementation
 -->
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:xsd="http://www.w3.org/2001/XMLSchema" xmlns:soap="http://schemas.xmlsoap.org/wsdl/soap/" xmlns:http="http://schemas.xmlsoap.org/wsdl/http/" xmlns:mime="http://schemas.xmlsoap.org/wsdl/mime/" xmlns:wsdl="http://schemas.xmlsoap.org/wsdl/">
    <xsl:output method="text" omit-xml-declaration="yes" indent="no"/>
<xsl:template match="esxdl">package <xsl:value-of select="EsdlService/@name"/>;
import java.util.*;
import java.math.*;
import org.xmlpull.v1.XmlPullParser;
import org.xmlpull.v1.XmlPullParserException;
import org.xmlpull.v1.XmlPullParserFactory;
import java.io.StringReader;
import java.lang.reflect.Constructor;
import java.lang.reflect.InvocationTargetException;
import java.io.IOException;

class EsdlContext
{
    String username;
    Integer clientMajorVersion;
    Integer clientMinorVersion;
}

<xsl:apply-templates select="EsdlEnumType"/>
<xsl:apply-templates select="EsdlStruct"/>
<xsl:apply-templates select="EsdlRequest"/>
<xsl:apply-templates select="EsdlResponse"/>
<xsl:apply-templates select="EsdlService"/>
</xsl:template>

<xsl:template match="EsdlStruct|EsdlRequest|EsdlResponse">
class <xsl:value-of select="@name"/><xsl:if test="@base_type"> extends <xsl:value-of select="@base_type"/></xsl:if>
{
<xsl:apply-templates select="EsdlElement|EsdlArray|EsdlEnum"/>}
</xsl:template>

<xsl:template name="outputJavaPrimitive">
        <xsl:param name="typename"/>
        <xsl:choose>
            <xsl:when test="$typename='bool'"><xsl:value-of select="'Boolean'"/></xsl:when>
            <xsl:when test="$typename='boolean'"><xsl:value-of select="'Boolean'"/></xsl:when>
            <xsl:when test="$typename='decimal'"><xsl:value-of select="'Float'"/></xsl:when> <!-- TODO: should be BigDecimal. Temporary to make old way work -->
            <xsl:when test="$typename='float'"><xsl:value-of select="'Float'"/></xsl:when>
            <xsl:when test="$typename='double'"><xsl:value-of select="'Double'"/></xsl:when>
            <xsl:when test="$typename='integer'"><xsl:value-of select="'Integer'"/></xsl:when>
            <xsl:when test="$typename='int64'"><xsl:value-of select="'BigInteger'"/></xsl:when>
            <xsl:when test="$typename='long'"><xsl:value-of select="'Integer'"/></xsl:when>
            <xsl:when test="$typename='int'"><xsl:value-of select="'Integer'"/></xsl:when>
            <xsl:when test="$typename='short'"><xsl:value-of select="'Short'"/></xsl:when>
            <xsl:when test="$typename='nonPositiveInteger'"><xsl:value-of select="'Integer'"/></xsl:when>
            <xsl:when test="$typename='negativeInteger'"><xsl:value-of select="'Integer'"/></xsl:when>
            <xsl:when test="$typename='nonNegativeInteger'"><xsl:value-of select="'Integer'"/></xsl:when>
            <xsl:when test="$typename='unsigned'"><xsl:value-of select="'Integer'"/></xsl:when>
            <xsl:when test="$typename='unsignedLong'"><xsl:value-of select="'Integer'"/></xsl:when>
            <xsl:when test="$typename='unsignedInt'"><xsl:value-of select="'Integer'"/></xsl:when>
            <xsl:when test="$typename='unsignedShort'"><xsl:value-of select="'Short'"/></xsl:when> <!-- TODO: should be Integer. Temporary to make old way work -->
            <xsl:when test="$typename='unsignedByte'"><xsl:value-of select="'Byte'"/></xsl:when>
            <xsl:when test="$typename='positiveInteger'"><xsl:value-of select="'Integer'"/></xsl:when>
            <xsl:when test="$typename='base64Binary'"><xsl:value-of select="'String'"/></xsl:when>
            <xsl:when test="$typename='string'"><xsl:value-of select="'String'"/></xsl:when>
            <xsl:when test="$typename='xsdString'"><xsl:value-of select="'String'"/></xsl:when>
            <xsl:when test="$typename='normalizedString'"><xsl:value-of select="'String'"/></xsl:when>
            <xsl:when test="$typename='binary'"><xsl:value-of select="'byte[]'"/></xsl:when>
            <xsl:otherwise><xsl:value-of select="$typename"/></xsl:otherwise>
        </xsl:choose>
</xsl:template>

<xsl:template name="checkTypeCategory">
        <xsl:param name="typename"/>
        <xsl:choose>
            <xsl:when test="/esxdl/EsdlStruct[@name=$typename]">ESPTypeCategory.STRUCT</xsl:when>
            <xsl:when test="/esxdl/EsdlSRequest[@name=$typename]">ESPTypeCategory.STRUCT</xsl:when>
            <xsl:when test="/esxdl/EsdlResponse[@name=$typename]">ESPTypeCategory.STRUCT</xsl:when>
            <xsl:when test="/esxdl/EsdlEnumType[@name=$typename]">ESPTypeCategory.ENUM</xsl:when>
            <xsl:when test="$typename='binary'">ESPTypeCategory.BINARY</xsl:when>
            <xsl:otherwise>ESPTypeCategory.PRIMITIVE</xsl:otherwise>
        </xsl:choose>
</xsl:template>

<xsl:template match="EsdlArray">
        <xsl:variable name="primitive">
            <xsl:call-template name="outputJavaPrimitive">
               <xsl:with-param name="typename">
            <xsl:choose>
                <xsl:when test="@type"><xsl:value-of select="@type"/></xsl:when>
                <xsl:when test="@complex_type"><xsl:value-of select="@complex_type"/></xsl:when>
            </xsl:choose>
               </xsl:with-param>
            </xsl:call-template>
        </xsl:variable>
            <xsl:text>    public </xsl:text>ArrayList&lt;<xsl:value-of select="$primitive"/>&gt;<xsl:text> </xsl:text><xsl:value-of select="@name"/>=new ArrayList&lt;<xsl:value-of select="$primitive"/>&gt;();<xsl:text>
</xsl:text>
</xsl:template>

<xsl:template match="EsdlElement">
        <xsl:variable name="primitive">
            <xsl:call-template name="outputJavaPrimitive">
               <xsl:with-param name="typename">
            <xsl:choose>
                <xsl:when test="@type"><xsl:value-of select="@type"/></xsl:when>
                <xsl:when test="@complex_type"><xsl:value-of select="@complex_type"/></xsl:when>
            </xsl:choose>
               </xsl:with-param>
            </xsl:call-template>
        </xsl:variable>
        <xsl:variable name="useQuotes">
          <xsl:choose>
            <xsl:when test="$primitive='String'"><xsl:value-of select="true()"/></xsl:when>
            <xsl:otherwise><xsl:value-of select="false()"/></xsl:otherwise>
          </xsl:choose>
        </xsl:variable>
        <xsl:text>    public </xsl:text>
    <xsl:value-of select="$primitive"/>
        <xsl:text> </xsl:text>
        <xsl:value-of select="@name"/>
        <xsl:choose>
            <xsl:when test="@type='binary'"></xsl:when>
            <xsl:when test="@default">
        <xsl:text> = new </xsl:text><xsl:value-of select="$primitive"/><xsl:text>(</xsl:text>
        <xsl:choose>
                <xsl:when test="$useQuotes">"<xsl:value-of select="@default"/>"</xsl:when>
                <xsl:when test="$primitive='Boolean'">
            <xsl:choose>
                <xsl:when test="@default='true'"><xsl:value-of select="'true'"/></xsl:when>
                <xsl:when test="@default='1'"><xsl:value-of select="'true'"/></xsl:when>
                <xsl:otherwise><xsl:value-of select="'false'"/></xsl:otherwise>
                    </xsl:choose>
                    </xsl:when>
                <xsl:otherwise><xsl:value-of select="@default"/></xsl:otherwise>
                </xsl:choose>
        <xsl:text>)</xsl:text>
            </xsl:when>
        </xsl:choose>
        <xsl:text>;
</xsl:text>
</xsl:template>

<xsl:template match="EsdlEnum">
        <xsl:variable name="enum_type" select="@enum_type"/>
        <xsl:variable name="primitive">
        <xsl:call-template name="outputJavaPrimitive">
               <xsl:with-param name="typename">
                     <xsl:value-of select="@enum_type"/>
               </xsl:with-param>
        </xsl:call-template>
        </xsl:variable>
        <xsl:text>    public </xsl:text>
    <xsl:value-of select="$primitive"/>
        <xsl:text> </xsl:text>
        <xsl:value-of select="@name"/>
            <xsl:if test="@default">
                <xsl:text> = </xsl:text><xsl:value-of select="$primitive"/>.fromString<xsl:text>(</xsl:text>"<xsl:value-of select="@default"/>")</xsl:if>
        <xsl:text>;
</xsl:text>
</xsl:template>

<xsl:template match="EsdlService">
public class <xsl:value-of select="@name"/>ServiceBase
{
    static XmlPullParserFactory factory = null;
    private synchronized void createFactory() throws XmlPullParserException
    {
        if (factory == null) {
            factory = XmlPullParserFactory.newInstance();
            factory.setNamespaceAware(true);
        }
    }
<xsl:call-template name="generateContextMethods"/>
<xsl:call-template name="generateCommonMethods"/>
    <xsl:for-each select="EsdlMethod">
    public <xsl:value-of select="@response_type"/><xsl:text> </xsl:text><xsl:value-of select="@name"/>(EsdlContext context, <xsl:value-of select="@request_type"/> request){return null;}
<xsl:call-template name="generateBaseMethod">
            <xsl:with-param name="name" select="@name"/>
            <xsl:with-param name="request_type" select="@request_type"/>
            <xsl:with-param name="response_type" select="@response_type"/>
    </xsl:call-template>
    </xsl:for-each>

    <xsl:for-each select="/esxdl/EsdlRequest|/esxdl/EsdlResponse|/esxdl/EsdlStruct">
        <xsl:call-template name="generateUnserializeForStruct"/>
    </xsl:for-each>

    <xsl:for-each select="/esxdl/EsdlEnumType">
        <xsl:call-template name="generateUnserializeForEnum"/>
    </xsl:for-each>

    <xsl:for-each select="/esxdl/EsdlRequest|/esxdl/EsdlResponse|/esxdl/EsdlStruct">
        <xsl:call-template name="generateSerializeForStruct"/>
    </xsl:for-each>

}
</xsl:template>

<xsl:template match="EsdlEnumType">
      <xsl:if test="EsdlEnumItem">
enum <xsl:value-of select="@name"/><xsl:text> {
</xsl:text>
        <xsl:for-each select="EsdlEnumItem">
          <xsl:text>        </xsl:text><xsl:value-of select="@name"/><xsl:text> </xsl:text>("<xsl:value-of select="@enum"/><xsl:text>")</xsl:text>
           <xsl:choose>
             <xsl:when test="position() != last()">
              <xsl:text>,
</xsl:text>
             </xsl:when>
             <xsl:otherwise>
              <xsl:text>;
</xsl:text>
             </xsl:otherwise>
           </xsl:choose>
        </xsl:for-each>
    private final String name;

    private<xsl:text> </xsl:text><xsl:value-of select="@name"/>(String s) {
        name = s;
    }

    public boolean equalsName(String otherName) {
       return (otherName == null) ? false : name.equals(otherName);
    }

    public String toString() {
        return this.name;
    }
    public static<xsl:text> </xsl:text><xsl:value-of select="@name"/> fromString(String text)
    {
        if (text != null)
        {
            for (<xsl:value-of select="@name"/> val :<xsl:text> </xsl:text><xsl:value-of select="@name"/>.values()) {
                if (text.equalsIgnoreCase(val.toString())) {
                    return val;
                }
            }
        }
        return null;
    }
}
</xsl:if>
</xsl:template>

<!-- TODO: automate by generating from esxdl-like representation -->
<xsl:template name="generateContextMethods">
    public EsdlContext unserializeEsdlContext(String CtxXML)
            throws XmlPullParserException, IOException {
        if(CtxXML == null)
            return null;
        createFactory();
        XmlPullParser xpp = factory.newPullParser();
        xpp.setInput(new StringReader(CtxXML));

        EsdlContext structobj = new EsdlContext();
        int eventType = xpp.next();
        while (eventType != XmlPullParser.END_DOCUMENT)
        {
            if (eventType == XmlPullParser.START_TAG)
            {
                String tagname = xpp.getName();
                if (tagname.equals("EsdlContext"))
                {
                    int[] Level = new int[1];
                    Level[0] = 0;
                    eventType = unserializeEsdlContext(Level, xpp, structobj);
                    break;
                }
            }
            eventType = xpp.next();
        }
        return structobj;
    }

    public int unserializeEsdlContext(int[] Level, XmlPullParser xpp, EsdlContext structobj)
            throws XmlPullParserException, IOException
    {
        int L = Level[0];
        int eventType = xpp.next();
        while (eventType != XmlPullParser.END_DOCUMENT &amp;&amp; Level[0] >= L) {
            if (eventType == XmlPullParser.START_TAG) {
                Level[0]++;
                String tagname = xpp.getName();
                if (tagname.equals("username"))
                {
                    ESPUnserializeResultPair&lt;String&gt; result = unserializePrimitive(Level, xpp, String.class);
                    eventType = result.event;
                    structobj.username = result.value;
                }
            }
            else if(eventType == XmlPullParser.END_TAG)
            {
                Level[0]--;
            }
            if (eventType != XmlPullParser.END_DOCUMENT &amp;&amp; Level[0] >= L)
                eventType = xpp.next();
        }
        return eventType;
    }
</xsl:template>

<xsl:template name="generateCommonMethods">
    class ESPUnserializeResultPair &lt;T&gt;
    {
        protected int event;
        protected T value;
    }

    enum ESPTypeCategory
    {
        PRIMITIVE,
        STRUCT,
        ENUM,
        BINARY
    }

    public &lt;T&gt; ESPUnserializeResultPair&lt;T&gt; unserializePrimitive(int[] Level, XmlPullParser xpp, Class&lt;T&gt; valueType)
            throws XmlPullParserException, IOException
    {
        ESPUnserializeResultPair&lt;T&gt; result = new ESPUnserializeResultPair&lt;T&gt;();
        result.value = null;
        int eventType = xpp.next();
        int L = Level[0];
        while (eventType != XmlPullParser.END_DOCUMENT &amp;&amp; Level[0] >= L)
        {
            if (eventType == XmlPullParser.TEXT)
            {
                String valstr = xpp.getText();
                if(valstr != null)
                {
                    valstr.trim();
                    if(valstr.length() > 0)
                    {
                        try
                        {
                            Constructor&lt;T&gt; ctor = valueType.getConstructor(String.class);
                            result.value = (T) (ctor.newInstance(valstr));
                        }
                        catch (InstantiationException | IllegalAccessException | IllegalArgumentException
                                | InvocationTargetException | NoSuchMethodException | SecurityException e)
                        {
                            // TODO Auto-generated catch block
                            e.printStackTrace();
                        }
                    }
                }
            }
            else if (eventType == XmlPullParser.START_TAG)
            {
                Level[0]++;
            }
            else if (eventType == XmlPullParser.END_TAG)
            {
                Level[0]--;
            }
            if (eventType != XmlPullParser.END_DOCUMENT &amp;&amp; Level[0] >= L)
                eventType = xpp.next();
        }
        result.event = eventType;
        return result;
    }

    public &lt;T&gt; ESPUnserializeResultPair&lt;byte[]&gt; unserializeBinary(int[] Level, XmlPullParser xpp)
            throws XmlPullParserException, IOException
    {
        ESPUnserializeResultPair&lt;byte[]&gt; result = new ESPUnserializeResultPair&lt;byte[]&gt;();
        result.value = null;
        int eventType = xpp.next();
        int L = Level[0];
        while (eventType != XmlPullParser.END_DOCUMENT &amp;&amp; Level[0] >= L)
        {
            if (eventType == XmlPullParser.TEXT)
            {
                String valstr = xpp.getText();
                if(valstr != null)
                {
                    valstr.trim();
                    if(valstr.length() > 0)
                    {
                        result.value = valstr.getBytes();
                    }
                }
            }
            else if (eventType == XmlPullParser.START_TAG)
            {
                Level[0]++;
            }
            else if (eventType == XmlPullParser.END_TAG)
            {
                Level[0]--;
            }
            if (eventType != XmlPullParser.END_DOCUMENT &amp;&amp; Level[0] >= L)
                eventType = xpp.next();
        }
        result.event = eventType;
        return result;
    }


    public &lt;T&gt; ESPUnserializeResultPair&lt;ArrayList&lt;T&gt;&gt; unserializeArrayList(int[] Level, XmlPullParser xpp, String itemName, String itemTypeName, ESPTypeCategory itemTypeCategory, Class&lt;T&gt; itemType)
            throws XmlPullParserException, IOException
    {
        java.lang.reflect.Method method = null;
        if(itemTypeCategory == ESPTypeCategory.STRUCT || itemTypeCategory == ESPTypeCategory.ENUM)
        {
            try {
                if(itemTypeCategory == ESPTypeCategory.STRUCT)
                    method = this.getClass().getMethod("unserialize"+itemTypeName, Level.getClass(), XmlPullParser.class, itemType);
                else if(itemTypeCategory == ESPTypeCategory.ENUM)
                    method = this.getClass().getMethod("unserializeEnum"+itemTypeName, Level.getClass(), XmlPullParser.class);
            }
            catch (SecurityException e)
            {
                e.printStackTrace();
            }
            catch(NoSuchMethodException e)
            {
                e.printStackTrace();
            }
            if(method == null)
                return null;
        }
        ArrayList&lt;T&gt; arr = new ArrayList&lt;T&gt;();
        int eventType = xpp.next();
        int L = Level[0];
        while (eventType != XmlPullParser.END_DOCUMENT &amp;&amp; Level[0] >= L)
        {
            if (eventType == XmlPullParser.START_TAG)
            {
                Level[0]++;
                String tagname = xpp.getName();
                if (tagname.equals(itemName))
                {
                    if(itemTypeCategory == ESPTypeCategory.STRUCT) //EsdlStruct
                    {
                        T newstructobj = null;
                        try
                        {
                            newstructobj = itemType.newInstance();
                        }
                        catch (InstantiationException | IllegalAccessException e)
                        {
                            // TODO Auto-generated catch block
                            e.printStackTrace();
                        }
                        if(newstructobj != null)
                        {
                            arr.add(newstructobj);
                            try
                            {
                                Object o = method.invoke(this, Level, xpp, newstructobj);
                            }
                            catch (IllegalAccessException | IllegalArgumentException | InvocationTargetException e)
                            {
                                // TODO Auto-generated catch block
                                e.printStackTrace();
                            }
                        }
                    }
                    else if(itemTypeCategory == ESPTypeCategory.ENUM) //EsdlEnum
                    {
                        try
                        {
                            @SuppressWarnings("unchecked")
                            ESPUnserializeResultPair&lt;T&gt; result = (ESPUnserializeResultPair&lt;T&gt;) method.invoke(this, Level, xpp);
                            eventType = result.event;
                            arr.add(result.value);
                        }
                        catch (IllegalAccessException | IllegalArgumentException
                                | InvocationTargetException | SecurityException e)
                        {
                            // TODO Auto-generated catch block
                            e.printStackTrace();
                        }
                    }
                    else if(itemTypeCategory == ESPTypeCategory.BINARY)
                    {
                        ESPUnserializeResultPair&lt;T&gt; result = (ESPUnserializeResultPair&lt;T&gt;)unserializeBinary(Level, xpp);
                        eventType = result.event;
                        arr.add(result.value);
                    }
                    else //Primitive
                    {
                        ESPUnserializeResultPair&lt;T&gt; result = unserializePrimitive(Level, xpp, itemType);
                        eventType = result.event;
                        arr.add(result.value);
                    }
                }
            }
            else if (eventType == XmlPullParser.END_TAG)
            {
                Level[0]--;
            }
            if (eventType != XmlPullParser.END_DOCUMENT &amp;&amp; Level[0] >= L)
                eventType = xpp.next();
        }
        ESPUnserializeResultPair&lt;ArrayList&lt;T&gt;&gt; result = new ESPUnserializeResultPair&lt;ArrayList&lt;T&gt;&gt;();
        result.event = eventType;
        result.value = arr;
        return result;
    }
</xsl:template>

<xsl:template name="generateBaseMethod">
        <xsl:param name="name"/>
        <xsl:param name="request_type"/>
        <xsl:param name="response_type"/>
    public String <xsl:value-of select="$name"/>(String CtxXML, String ReqXML) {
        long starttime = System.nanoTime();
        String respXML = null;
        try {
            EsdlContext ctx = unserializeEsdlContext(CtxXML);
            <xsl:value-of select="$request_type"/> request = unserialize<xsl:value-of select="$request_type"/>(ReqXML);
            <xsl:value-of select="$response_type"/> response = <xsl:value-of select="$name"/>(ctx, request);
            StringBuilder respbuilder = new StringBuilder();
            respbuilder.append("&lt;Response&gt;&lt;Results&gt;&lt;Result&gt;&lt;Dataset name=\"").append("<xsl:value-of select="$response_type"/>")
                .append("\"&gt;&lt;Row&gt;");
            serialize<xsl:value-of select="$response_type"/>(null, response, respbuilder);
            respbuilder.append("&lt;/Row&gt;&lt;/Dataset&gt;&lt;/Result&gt;&lt;/Results&gt;&lt;/Response&gt;");
            respXML = respbuilder.toString();
        } catch (Exception e) {
            System.out.println("Exception: " + e);
            return e.toString();
        }
        long stoptime = System.nanoTime();
        System.out.println("Time taken: " + (stoptime - starttime) / 1000000 + " milli seconds\n");
        return respXML;
    }

    public <xsl:value-of select="$request_type"/> unserialize<xsl:value-of select="$request_type"/>(String ReqXML)
            throws XmlPullParserException, IOException {
        if(ReqXML == null)
            return null;

        createFactory();
        long starttime = System.nanoTime();
        XmlPullParser xpp = factory.newPullParser();
        xpp.setInput(new StringReader(ReqXML));

        <xsl:value-of select="$request_type"/> structobj = new <xsl:value-of select="$request_type"/>();
        int eventType = xpp.next();
        while (eventType != XmlPullParser.END_DOCUMENT)
        {
            if (eventType == XmlPullParser.START_TAG)
            {
                String tagname = xpp.getName();
                if (tagname.equals("<xsl:value-of select="$request_type"/>"))
                {
                    int[] Level = new int[1];
                    Level[0] = 0;
                    eventType = unserialize<xsl:value-of select="$request_type"/>(Level, xpp, structobj);
                    break;
                }
            }
            eventType = xpp.next();
        }
        long stoptime = System.nanoTime();
        System.out.println("Time taken unserialize: " + (stoptime - starttime) / 1000000 + " milli seconds");
        return structobj;
    }
</xsl:template>

<xsl:template name="generateUnserializeForStruct">
    public int unserialize<xsl:value-of select="@name"/>(int[] Level, XmlPullParser xpp, <xsl:value-of select="@name"/> structobj)
            throws XmlPullParserException, IOException
    {
        int L = Level[0];
        int eventType = xpp.next();
        while (eventType != XmlPullParser.END_DOCUMENT &amp;&amp; Level[0] >= L) {
            if (eventType == XmlPullParser.START_TAG) {
                Level[0]++;
                String tagname = xpp.getName();<xsl:for-each select="./*"><xsl:choose><xsl:when test="position()=1">
                if(tagname.equals("<xsl:value-of select="@name"/>"))</xsl:when>
                    <xsl:otherwise>
                else if(tagname.equals("<xsl:value-of select="@name"/>"))</xsl:otherwise></xsl:choose>
                {<xsl:text></xsl:text>
                    <xsl:choose>
                        <xsl:when test="name()='EsdlElement'">
                            <xsl:choose>
                                <xsl:when test="@type">
                                <xsl:choose>
                                <xsl:when test="@type='binary'">
                   ESPUnserializeResultPair&lt;byte[]&gt; result = unserializeBinary(Level, xpp);
                   eventType = result.event;
                   structobj.<xsl:value-of select="@name"/>=result.value;<xsl:text></xsl:text>
                                </xsl:when>
                                <xsl:otherwise>
                                    <xsl:variable name="javaTypeName">
                                        <xsl:call-template name="outputJavaPrimitive">
                                            <xsl:with-param name="typename">
                                                <xsl:value-of select="@type"/>
                                            </xsl:with-param>
                                        </xsl:call-template>
                                    </xsl:variable>
                    ESPUnserializeResultPair&lt;<xsl:value-of select="$javaTypeName"/>&gt; result = unserializePrimitive(Level, xpp, <xsl:value-of select="$javaTypeName"/>.class);
                    eventType = result.event;
                    structobj.<xsl:value-of select="@name"/>=result.value;<xsl:text></xsl:text>
                                </xsl:otherwise>
                                </xsl:choose>
                                </xsl:when>
                                <xsl:when test="@complex_type"><xsl:text>
                    </xsl:text>
                    <xsl:value-of select="@complex_type"/> newstruct = new <xsl:value-of select="@complex_type"/>();
                    structobj.<xsl:value-of select="@name"/> = newstruct;
                    unserialize<xsl:value-of select="@complex_type"/>(Level, xpp, newstruct);</xsl:when></xsl:choose>
                        </xsl:when>
                        <xsl:when test="name()='EsdlEnum'">
                    ESPUnserializeResultPair&lt;<xsl:value-of select="@enum_type"/>&gt; result = unserializeEnum<xsl:value-of select="@enum_type"/>(Level, xpp);
                    eventType = result.event;
                    structobj.<xsl:value-of select="@name"/>=result.value;</xsl:when>
                        <xsl:when test="name()='EsdlArray'">
                            <xsl:variable name="typeCategory">
                                <xsl:call-template name="checkTypeCategory">
                                    <xsl:with-param name="typename">
                                        <xsl:value-of select="@type"/>
                                    </xsl:with-param>
                                </xsl:call-template>
                            </xsl:variable>
                            <xsl:variable name="javaTypeName">
                                <xsl:call-template name="outputJavaPrimitive">
                                    <xsl:with-param name="typename">
                                        <xsl:value-of select="@type"/>
                                    </xsl:with-param>
                                </xsl:call-template>
                            </xsl:variable>
                    ESPUnserializeResultPair&lt;ArrayList&lt;<xsl:value-of select="$javaTypeName"/>&gt;&gt; result = this.unserializeArrayList(Level, xpp, "<xsl:value-of select="@item_tag"/>", "<xsl:value-of select="$javaTypeName"/>", <xsl:value-of select="$typeCategory"/>, <xsl:value-of select="$javaTypeName"/>.class);
                    structobj.<xsl:value-of select="@name"/> = result.value;
                    eventType = result.event;</xsl:when>
                    </xsl:choose>
                }</xsl:for-each>
            }
            else if(eventType == XmlPullParser.END_TAG)
            {
                Level[0]--;
            }
            if (eventType != XmlPullParser.END_DOCUMENT &amp;&amp; Level[0] >= L)
                eventType = xpp.next();
        }
        return eventType;
    }
</xsl:template>

<xsl:template name="generateUnserializeForEnum">
    public ESPUnserializeResultPair&lt;<xsl:value-of select="@name"/>&gt; unserializeEnum<xsl:value-of select="@name"/>(int[] Level, XmlPullParser xpp)
            throws XmlPullParserException, IOException
    {
        ESPUnserializeResultPair&lt;<xsl:value-of select="@name"/>&gt; result = new ESPUnserializeResultPair&lt;<xsl:value-of select="@name"/>&gt;();
        result.value = null;
        int L = Level[0];
        int eventType = xpp.next();

        while (eventType != XmlPullParser.END_DOCUMENT &amp;&amp; Level[0] >= L) {
            if (eventType == XmlPullParser.TEXT)
            {
                String valstr = xpp.getText();
                if (valstr != null)
                {
                    valstr = valstr.trim();
                    if(valstr.length() > 0)
                    {
                        result.value = <xsl:value-of select="@name"/>.fromString(valstr);
                    }
                }
            }
            else if (eventType == XmlPullParser.START_TAG)
            {
                Level[0]++;
            }
            else if (eventType == XmlPullParser.END_TAG)
            {
                Level[0]--;
            }
            if (eventType != XmlPullParser.END_DOCUMENT &amp;&amp; Level[0] >= L)
                eventType = xpp.next();
        }
        result.event = eventType;
        return result;
    }
</xsl:template>

<xsl:template name="generateSerializeForStruct">
    public void serialize<xsl:value-of select="@name"/>(String tagname, <xsl:value-of select="@name"/> structobj, StringBuilder resultbuf)
    {
        if(tagname != null &amp;&amp; tagname.length() > 0)
            resultbuf.append("&lt;").append(tagname).append("&gt;");<xsl:text></xsl:text>
        <xsl:for-each select="./*">
        if(structobj.<xsl:value-of select="@name"/> != null)
        {<xsl:choose>
            <xsl:when test="name()='EsdlElement'">
                <xsl:choose>
                    <xsl:when test="@type">
            resultbuf.append("&lt;").append("<xsl:value-of select="@name"/>").append("&gt;");<xsl:text></xsl:text>
                    <xsl:choose>
                    <xsl:when test="@type='binary'">
            resultbuf.append(new String(structobj.<xsl:value-of select="@name"/>));<xsl:text></xsl:text>
                    </xsl:when>
                    <xsl:otherwise>
            resultbuf.append(structobj.<xsl:value-of select="@name"/>);<xsl:text></xsl:text>
                    </xsl:otherwise>
            resultbuf.append("&lt;/").append("<xsl:value-of select="@name"/>").append("&gt;");<xsl:text></xsl:text>
                    </xsl:choose>
                    </xsl:when>
                    <xsl:when test="@complex_type">
            serialize<xsl:value-of select="@complex_type"/>("<xsl:value-of select="@name"/>", structobj.<xsl:value-of select="@name"/>, resultbuf);<xsl:text></xsl:text>
                    </xsl:when>
                </xsl:choose>
            </xsl:when>
            <xsl:when test="name()='EsdlEnum'">
            resultbuf.append("&lt;").append("<xsl:value-of select="@name"/>").append("&gt;");
            resultbuf.append(structobj.<xsl:value-of select="@name"/>.toString());
            resultbuf.append("&lt;/").append("<xsl:value-of select="@name"/>").append("&gt;");<xsl:text></xsl:text>
            </xsl:when>
            <xsl:when test="name()='EsdlArray'">
            resultbuf.append("&lt;").append("<xsl:value-of select="@name"/>").append("&gt;");<xsl:text></xsl:text>
            <xsl:variable name="javaTypeName">
                <xsl:call-template name="outputJavaPrimitive">
                    <xsl:with-param name="typename">
                        <xsl:value-of select="@type"/>
                    </xsl:with-param>
                </xsl:call-template>
            </xsl:variable>
            <xsl:variable name="typeCategory">
                <xsl:call-template name="checkTypeCategory">
                    <xsl:with-param name="typename">
                        <xsl:value-of select="@type"/>
                    </xsl:with-param>
                </xsl:call-template>
            </xsl:variable>
            for(<xsl:value-of select="$javaTypeName"/> curobj:structobj.<xsl:value-of select="@name"/>)
            {<xsl:text></xsl:text>
                <xsl:choose>
                   <xsl:when test="$typeCategory='ESPTypeCategory.STRUCT'">
                serialize<xsl:value-of select="@type"/>("<xsl:value-of select="@item_tag"/>", curobj, resultbuf);
                   </xsl:when>
                   <xsl:when test="typeCatgory='ESPTypeCategory.ENUM'">
                resultbuf.append("&lt;").append("<xsl:value-of select="@item_tag"/>").append("&gt;")
                         .append(curobj.toString()).append("&lt;/").append("<xsl:value-of select="@item_tag"/>").append("&gt;");<xsl:text></xsl:text>
                   </xsl:when>
                   <xsl:when test="@type='binary'">
                resultbuf.append("&lt;").append("<xsl:value-of select="@item_tag"/>").append("&gt;")
                         .append(new String(curobj)).append("&lt;/").append("<xsl:value-of select="@item_tag"/>").append("&gt;");<xsl:text></xsl:text>
                   </xsl:when>
                   <xsl:otherwise>
                resultbuf.append("&lt;").append("<xsl:value-of select="@item_tag"/>").append("&gt;")
                         .append(curobj).append("&lt;/").append("<xsl:value-of select="@item_tag"/>").append("&gt;");<xsl:text></xsl:text>
                   </xsl:otherwise>
                </xsl:choose>
            }
            resultbuf.append("&lt;/").append("<xsl:value-of select="@name"/>").append("&gt;");<xsl:text></xsl:text>
            </xsl:when>
        </xsl:choose>
        }</xsl:for-each>
        if(tagname != null &amp;&amp; tagname.length() > 0)
            resultbuf.append("&lt;/").append(tagname).append("&gt;");
    }
</xsl:template>
</xsl:stylesheet>
