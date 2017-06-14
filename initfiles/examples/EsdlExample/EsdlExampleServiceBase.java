package EsdlExample;
import java.util.*;

import org.xmlpull.v1.XmlPullParser;
import org.xmlpull.v1.XmlPullParserException;
import org.xmlpull.v1.XmlPullParserFactory;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.StringReader;
import java.lang.reflect.Constructor;
import java.lang.reflect.InvocationTargetException;
import java.math.*;

class EsdlContext
{
    String username;
    Integer clientMajorVersion;
    Integer clientMinorVersion;
}


    enum AddressType {
        HOME ("Home"),
        WORK ("Work"),
        HOTEL ("Hotel");

            private final String name;

            private AddressType(String s) {
                name = s;
            }

            public boolean equalsName(String otherName) {
                return (otherName == null) ? false : name.equals(otherName);
            }

            public String toString() {
               return this.name;
            }
            public static AddressType fromString(String text)
            {
              if (text != null)
              {
                  for (AddressType val : AddressType.values()) {
                    if (text.equalsIgnoreCase(val.toString())) {
                      return val;
                    }
                  }
               }
             return null;
            }
        }

class NameInfo
{
    public String First = new String("Joe");
    public String Last = new String("Doe");
    public ArrayList<String> Aliases=new ArrayList<String>();
    public ArrayList<AddressType> AddressTypes=new ArrayList<AddressType>();
}

class AddressInfo
{
    public AddressType type = AddressType.fromString("Home")
            ;
    public String Line1;
    public String Line2;
    public String City;
    public String State;
    public Integer Zip = new Integer("33487");
}

class JavaEchoPersonInfoRequest
{
    public NameInfo Name;
    public ArrayList<AddressInfo> Addresses=new ArrayList<AddressInfo>();
}

class RoxieEchoPersonInfoRequest
{
    public NameInfo Name;
    public ArrayList<AddressInfo> Addresses=new ArrayList<AddressInfo>();
}

class JavaEchoPersonInfoResponse
{
    public Integer count = new Integer("0");
    public NameInfo Name;
    public ArrayList<AddressInfo> Addresses=new ArrayList<AddressInfo>();
}

class RoxieEchoPersonInfoResponse
{
    public Integer count = new Integer("0");
    public NameInfo Name;
    public ArrayList<AddressInfo> Addresses=new ArrayList<AddressInfo>();
}

public class EsdlExampleServiceBase
{

    public JavaEchoPersonInfoResponse JavaEchoPersonInfo(EsdlContext context, JavaEchoPersonInfoRequest request){return null;}

    public RoxieEchoPersonInfoResponse RoxieEchoPersonInfo(EsdlContext context, RoxieEchoPersonInfoRequest request){return null;}

    public String JavaEchoPersonInfo(String CtxXML, String ReqXML) {
        long starttime = System.nanoTime();
        String respXML = null;
        try {
            JavaEchoPersonInfoRequest request = unserializeJavaEchoPersonInfoRequest(ReqXML);
            EsdlContext ctx = unserializeEsdlContext(CtxXML);
            JavaEchoPersonInfoResponse response = JavaEchoPersonInfo(ctx, request);
            respXML = serializeResp(response);
        } catch (Exception e) {
            System.out.println("Exception: " + e);
            return e.toString();
        }
        long stoptime = System.nanoTime();
        System.out.println("Time taken: " + (stoptime - starttime) / 1000000 + " milli seconds\n");
        return respXML;
    }

    static XmlPullParserFactory factory = null;
    public synchronized void createFactory() throws XmlPullParserException {
        if (factory == null) {
            factory = XmlPullParserFactory.newInstance();
            factory.setNamespaceAware(true);
        }
    }

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
        while (eventType != XmlPullParser.END_DOCUMENT && Level[0] >= L) {
            if (eventType == XmlPullParser.START_TAG) {
                Level[0]++;
                String tagname = xpp.getName();
                if (tagname.equals("username"))
                {
                    ESPUnserializeResultPair<String> result = unserializePrimitive(Level, xpp);
                    eventType = result.event;
                    structobj.username = result.value;
                }
            }
            else if(eventType == XmlPullParser.END_TAG)
            {
                Level[0]--;
            }
            if (eventType != XmlPullParser.END_DOCUMENT && Level[0] >= L)
                eventType = xpp.next();
        }
        return eventType;
    }

    //TODO:
    // - Trim Text or not? (Roxie query does it. Should we leave it to the developer to decide?)
    public JavaEchoPersonInfoRequest unserializeJavaEchoPersonInfoRequest(String ReqXML)
            throws XmlPullParserException, IOException {
        if(ReqXML == null)
            return null;

        createFactory();
        long starttime = System.nanoTime();
        XmlPullParser xpp = factory.newPullParser();
        xpp.setInput(new StringReader(ReqXML));

        JavaEchoPersonInfoRequest structobj = new JavaEchoPersonInfoRequest();
        int eventType = xpp.next();
        while (eventType != XmlPullParser.END_DOCUMENT)
        {
            if (eventType == XmlPullParser.START_TAG)
            {
                String tagname = xpp.getName();
                if (tagname.equals("JavaEchoPersonInfoRequest"))
                {
                    int[] Level = new int[1];
                    Level[0] = 0;
                    eventType = unserializeJavaEchoPersonInfoRequest(Level, xpp, structobj);
                    break;
                }
            }
            eventType = xpp.next();
        }
        long stoptime = System.nanoTime();
        System.out.println("Time taken unserialize: " + (stoptime - starttime) / 1000000 + " milli seconds");
        return structobj;
    }

    public int unserializeJavaEchoPersonInfoRequest(int[] Level, XmlPullParser xpp, JavaEchoPersonInfoRequest structobj)
            throws XmlPullParserException, IOException
    {
        int L = Level[0];
        int eventType = xpp.next();
        while (eventType != XmlPullParser.END_DOCUMENT && Level[0] >= L) {
            if (eventType == XmlPullParser.START_TAG) {
                Level[0]++;
                String tagname = xpp.getName();
                if (tagname.equals("Name"))
                {
                    structobj.Name = new NameInfo();
                    eventType = unserializeName(Level, xpp, structobj.Name);
                }
                else if (tagname.equals("Addresses"))
                {
                    ESPUnserializeResultPair<ArrayList<AddressInfo>> result = this.unserializeArrayList(Level, xpp, "Address", "AddressInfo", ESPTypeCategory.STRUCT, AddressInfo.class);
                    structobj.Addresses = result.value;
                    eventType = result.event;
                }
            }
            else if(eventType == XmlPullParser.END_TAG)
            {
                Level[0]--;
            }
            if (eventType != XmlPullParser.END_DOCUMENT && Level[0] >= L)
                eventType = xpp.next();
        }
        return eventType;
    }

    public int unserializeName(int[] Level, XmlPullParser xpp, NameInfo structobj)
            throws XmlPullParserException, IOException
    {
        int eventType = xpp.next();
        int L = Level[0];
        while (eventType != XmlPullParser.END_DOCUMENT && Level[0] >= L)
        {
            if (eventType == XmlPullParser.START_TAG)
            {
                Level[0]++;
                String tagname = xpp.getName();
                if (tagname.equals("First"))
                {
                    ESPUnserializeResultPair<String> result = unserializePrimitive(Level, xpp);
                    eventType = result.event;
                    structobj.First = result.value;
                }
                else if (tagname.equals("Last"))
                {
                    ESPUnserializeResultPair<String> result = unserializePrimitive(Level, xpp);
                    eventType = result.event;
                    structobj.Last = result.value;
                }
                else if (tagname.equals("Aliases"))
                {
                    ESPUnserializeResultPair<ArrayList<String>> result = this.unserializeArrayList(Level, xpp, "Alias", "String", ESPTypeCategory.PRIMITIVE, String.class);
                    structobj.Aliases = result.value;
                    eventType = result.event;
                }
                else if (tagname.equals("AddressTypes"))
                {
                    ESPUnserializeResultPair<ArrayList<AddressType>> result = this.unserializeArrayList(Level, xpp, "AddrType", "AddressType", ESPTypeCategory.ENUM, AddressType.class);
                    structobj.AddressTypes = result.value;
                    eventType = result.event;
                }
            }
            else if (eventType == XmlPullParser.END_TAG)
            {
                Level[0]--;
            }
            if (eventType != XmlPullParser.END_DOCUMENT && Level[0] >= L)
                eventType = xpp.next();
        }
        return eventType;
    }

    public <T> ESPUnserializeResultPair<ArrayList<T>> unserializeArrayList(int[] Level, XmlPullParser xpp, String itemName, String itemTypeName, ESPTypeCategory itemTypeCategory, Class<T> itemType)
            throws XmlPullParserException, IOException
    {
        java.lang.reflect.Method method = null;;
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
        ArrayList<T> arr = new ArrayList<T>();
        int eventType = xpp.next();
        int L = Level[0];
        while (eventType != XmlPullParser.END_DOCUMENT && Level[0] >= L)
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
                            ESPUnserializeResultPair<T> result = (ESPUnserializeResultPair<T>) method.invoke(this, Level, xpp);
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
                    else //Primitive
                    {
                        ESPUnserializeResultPair<String> result = unserializePrimitive(Level, xpp);
                        eventType = result.event;
                        try
                        {
                            Constructor<T> ctor = itemType.getConstructor(String.class);
                            arr.add((T) (ctor.newInstance(result.value)));
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
            else if (eventType == XmlPullParser.END_TAG)
            {
                Level[0]--;
            }
            if (eventType != XmlPullParser.END_DOCUMENT && Level[0] >= L)
                eventType = xpp.next();
        }
        ESPUnserializeResultPair<ArrayList<T>> result = new ESPUnserializeResultPair<ArrayList<T>>();
        result.event = eventType;
        result.value = arr;
        return result;
    }

    public int unserializeAddressInfo(int[] Level, XmlPullParser xpp, AddressInfo structobj)
            throws XmlPullParserException, IOException
    {
        int eventType = xpp.next();
        int L = Level[0];
        while (eventType != XmlPullParser.END_DOCUMENT && Level[0] >= L)
        {
            if (eventType == XmlPullParser.START_TAG)
            {
                Level[0]++;
                String tagname = xpp.getName();
                if (tagname.equals("Line1"))
                {
                    ESPUnserializeResultPair<String> result = unserializePrimitive(Level, xpp);
                    eventType = result.event;
                    if(result.value != null)
                        structobj.Line1 = result.value;
                }
                else if (tagname.equals("Line2"))
                {
                    ESPUnserializeResultPair<String> result = unserializePrimitive(Level, xpp);
                    eventType = result.event;
                    if(result.value != null)
                        structobj.Line2 = result.value;
                }
                else if (tagname.equals("City"))
                {
                    ESPUnserializeResultPair<String> result = unserializePrimitive(Level, xpp);
                    eventType = result.event;
                    if(result.value != null)
                        structobj.City = result.value;
                }
                else if (tagname.equals("State"))
                {
                    ESPUnserializeResultPair<String> result = unserializePrimitive(Level, xpp);
                    eventType = result.event;
                    if(result.value != null)
                        structobj.State = result.value;
                }
                else if (tagname.equals("Zip"))
                {
                    ESPUnserializeResultPair<String> result = unserializePrimitive(Level, xpp);
                    eventType = result.event;
                    if(result.value != null)
                        structobj.Zip = new Integer(result.value);
                }
                else if (tagname.equals("type"))
                {
                    ESPUnserializeResultPair<AddressType> result = unserializeEnumAddressType(Level, xpp);
                    eventType = result.event;
                    structobj.type = result.value;
                }
            }
            else if (eventType == XmlPullParser.END_TAG)
            {
                Level[0]--;
            }
            if (eventType != XmlPullParser.END_DOCUMENT && Level[0] >= L)
                eventType = xpp.next();
        }
        return eventType;
    }

    class ESPUnserializeResultPair <T>
    {
        protected int event;
        protected T value;
    }

    enum ESPTypeCategory
    {
        PRIMITIVE,
        STRUCT,
        ENUM
    }

    public ESPUnserializeResultPair<AddressType> unserializeEnumAddressType(int[] Level, XmlPullParser xpp)
            throws XmlPullParserException, IOException
    {
        ESPUnserializeResultPair<AddressType> result = new ESPUnserializeResultPair<AddressType>();
        result.value = null;
        int L = Level[0];
        int eventType = xpp.next();

        while (eventType != XmlPullParser.END_DOCUMENT && Level[0] >= L) {
            if (eventType == XmlPullParser.TEXT)
            {
                String valstr = xpp.getText();
                if (valstr != null)
                {
                    valstr = valstr.trim();
                    if(valstr.length() > 0)
                    {
                        result.value = AddressType.fromString(valstr);
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
            if (eventType != XmlPullParser.END_DOCUMENT && Level[0] >= L)
                eventType = xpp.next();
        }
        result.event = eventType;
        return result;
    }

    public ESPUnserializeResultPair<String> unserializePrimitive(int[] Level, XmlPullParser xpp)
            throws XmlPullParserException, IOException
    {
        ESPUnserializeResultPair<String> result = new ESPUnserializeResultPair<String>();
        result.value = null;
        int eventType = xpp.next();
        int L = Level[0];
        while (eventType != XmlPullParser.END_DOCUMENT && Level[0] >= L)
        {
            if (eventType == XmlPullParser.TEXT)
            {
                String valstr = xpp.getText();
                if(valstr != null)
                {
                    valstr.trim();
                    if(valstr.length() > 0)
                    {
                        result.value = valstr;
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
            if (eventType != XmlPullParser.END_DOCUMENT && Level[0] >= L)
                eventType = xpp.next();
        }
        result.event = eventType;
        return result;
    }

    public String serializeResp(JavaEchoPersonInfoResponse resp) {
        long starttime = System.nanoTime();
        StringBuilder respstr = new StringBuilder();
        respstr.append("<Response><Results><Result><Dataset name=\"JavaEchoPersonInfoResponse\"><Row>");
        respstr.append("<count>").append(resp.count).append("</count>");
        if(resp.Name != null)
        {
            respstr.append("<Name>");
            if(resp.Name.First != null)
                respstr.append("<First>").append(resp.Name.First).append("</First>");
            if(resp.Name.Last != null)
            {
                respstr.append("<Last>").append(resp.Name.Last).append("</Last>");
            }
            if(resp.Name.Aliases != null)
            {
                respstr.append("<Aliases>");
                for(int i = 0; i < resp.Name.Aliases.size(); i++)
                {
                    String val = resp.Name.Aliases.get(i);
                    if(val != null)
                    {
                        respstr.append("<Alias>").append(val).append("</Alias>");
                    }
                }
                respstr.append("</Aliases>");
            }
            if(resp.Name.AddressTypes != null)
            {
                respstr.append("<AddressTypes>");
                for(int i = 0; i < resp.Name.AddressTypes.size(); i++)
                {
                    AddressType val = resp.Name.AddressTypes.get(i);
                    if(val != null)
                    {
                        respstr.append("<AddrType>").append(val).append("</AddrType>");
                    }
                }
                respstr.append("</AddressTypes>");
            }
            respstr.append("</Name>");
        }
        if (resp.Addresses != null)
        {
            respstr.append("<Addresses>");
            for (int i = 0; i < resp.Addresses.size(); i++)
            {
                AddressInfo addr = resp.Addresses.get(i);
                if(addr == null)
                    continue;
                respstr.append("<Address>");
                if (addr.Line1 != null)
                    respstr.append("<Line1>").append(addr.Line1).append("</Line1>");
                if (addr.Line2 != null)
                    respstr.append("<Line2>").append(addr.Line2).append("</Line2>");
                if (addr.City != null)
                    respstr.append("<City>").append(addr.City).append("</City>");
                if (addr.State != null)
                    respstr.append("<State>").append(addr.State).append("</State>");
                if(addr.Zip != null)
                    respstr.append("<Zip>").append(addr.Zip).append("</Zip>");
                if(addr.type != null)
                    respstr.append("<type>").append(addr.type).append("</type>");
                respstr.append("</Address>");
            }
            respstr.append("</Addresses>");
        }
        respstr.append("</Row></Dataset></Result></Results></Response>");
        long stoptime = System.nanoTime();
        System.out.println("Time taken serialize: " + (stoptime - starttime) / 1000000 + " milli seconds");
        return respstr.toString();
    }

    /*
     * Testing code below this line
     * ------------------------------------------------
     */
    public static void main(String args[])
    {
        if (args.length == 0)
        {
            System.out.println("Please specify xml file path");
            return;
        }
        try
        {
            String fname = args[0];
            String req = readFile(fname);
            EsdlExampleService service = new EsdlExampleService();
            service.testXPP(req);
            System.out.println("Start Processing " + fname);
            String resp = new String();
            for (int i = 0; i < 1; i++)
                resp = service.JavaEchoPersonInfo("<EsdlContext><username>joe doe</username></EsdlContext>", req);
            System.out.println("Finished Processing");
            System.out.println("Response:\n" + resp);

        }
        catch (Exception e)
        {
            System.out.println("Exception: " + e);
        }
    }

    protected void testXPP(String req) throws XmlPullParserException, IOException
    {
        createFactory();
        XmlPullParser xpp = factory.newPullParser();
        xpp.setInput(new StringReader(req));
        int eventType = xpp.getEventType();
        while (eventType != XmlPullParser.END_DOCUMENT)
        {
            if (eventType == XmlPullParser.START_DOCUMENT)
            {
                System.out.println("Start document");
            }
            else if (eventType == XmlPullParser.START_TAG)
            {
                System.out.println("Start tag " + xpp.getName());
            }
            else if (eventType == XmlPullParser.END_TAG)
            {
                System.out.println("End tag " + xpp.getName());
            }
            else if (eventType == XmlPullParser.TEXT)
            {
                System.out.println("Text " + xpp.getText());
            }
            eventType = xpp.next();
        }
        System.out.println("End document");
    }

    public static String readFile(String fpath) throws IOException
    {
        InputStream inputStream = new FileInputStream(fpath);
        long fileSize = new File(fpath).length();
        byte[] allBytes = new byte[(int) fileSize];
        inputStream.read(allBytes);
        inputStream.close();
        return new String(allBytes);
    }
}
