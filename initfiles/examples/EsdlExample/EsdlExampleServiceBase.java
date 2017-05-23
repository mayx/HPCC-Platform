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
            EsdlContext ctx = null; // unserializeContext(CtxXML);
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
    private synchronized void createFactory() throws XmlPullParserException {
        if (factory == null) {
            factory = XmlPullParserFactory.newInstance();
            factory.setNamespaceAware(true);
        }
    }

    //TODO:
    // - Is using Level the best way to do it?
    // - Trim Text or not?
    // - when serialize, how to decide whether to include numeric or boolean fields?
    private JavaEchoPersonInfoRequest unserializeJavaEchoPersonInfoRequest(String ReqXML)
            throws XmlPullParserException, IOException {
        createFactory();
        long starttime = System.nanoTime();
        XmlPullParser xpp = factory.newPullParser();
        xpp.setInput(new StringReader(ReqXML));

        JavaEchoPersonInfoRequest reqobj = new JavaEchoPersonInfoRequest();
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
                    eventType = unserializeJavaEchoPersonInfoRequest(Level, xpp, reqobj);
                    break;
                }
            }
            eventType = xpp.next();
        }
        long stoptime = System.nanoTime();
        System.out.println("Time taken unserialize: " + (stoptime - starttime) / 1000000 + " milli seconds");
        return reqobj;
    }

    private int unserializeJavaEchoPersonInfoRequest(int[] Level, XmlPullParser xpp, JavaEchoPersonInfoRequest reqobj)
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
                    reqobj.Name = new NameInfo();
                    eventType = unserializeName(Level, xpp, reqobj);
                }
                else if (tagname.equals("Addresses"))
                {
                    reqobj.Addresses = new ArrayList<AddressInfo>();
                    eventType = unserializeAddresses(Level, xpp, reqobj);
                }
            }
            else if(eventType == XmlPullParser.END_TAG)
            {
                Level[0]--;
            }
            if (eventType != XmlPullParser.END_DOCUMENT)
                eventType = xpp.next();
        }
        return eventType;
    }

    private int unserializeName(int[] Level, XmlPullParser xpp, JavaEchoPersonInfoRequest reqobj)
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
                    eventType = unserializeNameFirst(Level, xpp, reqobj);
                }
                else if (tagname.equals("Last"))
                {
                    eventType = unserializeNameLast(Level, xpp, reqobj);
                }
            }
            else if (eventType == XmlPullParser.END_TAG)
            {
                Level[0]--;
            }
            if (eventType != XmlPullParser.END_DOCUMENT)
                eventType = xpp.next();
        }
        return eventType;
    }

    private int unserializeNameFirst(int[] Level, XmlPullParser xpp, JavaEchoPersonInfoRequest reqobj)
            throws XmlPullParserException, IOException
    {
        int eventType = xpp.next();
        int L = Level[0];
        while (eventType != XmlPullParser.END_DOCUMENT && Level[0] >= L)
        {
            if (eventType == XmlPullParser.TEXT)
            {
                reqobj.Name.First = xpp.getText();
            }
            else if (eventType == XmlPullParser.START_TAG)
            {
                Level[0]++;
            }
            else if (eventType == XmlPullParser.END_TAG)
            {
                Level[0]--;
            }
            if (eventType != XmlPullParser.END_DOCUMENT)
                eventType = xpp.next();
        }
        return eventType;
    }

    private int unserializeNameLast(int[] Level, XmlPullParser xpp, JavaEchoPersonInfoRequest reqobj)
            throws XmlPullParserException, IOException
    {
        int eventType = xpp.next();
        int L = Level[0];
        while (eventType != XmlPullParser.END_DOCUMENT && Level[0] >= L)
        {
            if (eventType == XmlPullParser.TEXT)
            {
                reqobj.Name.Last = xpp.getText();
            }
            else if (eventType == XmlPullParser.START_TAG)
            {
                Level[0]++;
            }
            else if (eventType == XmlPullParser.END_TAG)
            {
                Level[0]--;
            }
            if (eventType != XmlPullParser.END_DOCUMENT)
                eventType = xpp.next();
        }
        return eventType;
    }

    private int unserializeAddresses(int[] Level, XmlPullParser xpp, JavaEchoPersonInfoRequest reqobj)
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
                if (tagname.equals("Address"))
                {
                    eventType = unserializeAddress(Level, xpp, reqobj);
                }
            }
            else if (eventType == XmlPullParser.END_TAG)
            {
                Level[0]--;
            }
            if (eventType != XmlPullParser.END_DOCUMENT)
                eventType = xpp.next();
        }
        return eventType;
    }

    private int unserializeAddress(int[] Level, XmlPullParser xpp, JavaEchoPersonInfoRequest reqobj)
            throws XmlPullParserException, IOException
    {
        int eventType = xpp.next();
        AddressInfo addr = new AddressInfo();
        reqobj.Addresses.add(addr);
        int L = Level[0];
        while (eventType != XmlPullParser.END_DOCUMENT && Level[0] >= L)
        {
            if (eventType == XmlPullParser.START_TAG)
            {
                Level[0]++;
                String tagname = xpp.getName();
                if (tagname.equals("Line1"))
                {
                    eventType = unserializeAddressLine1(Level, xpp, addr);
                }
                else if (tagname.equals("Line2"))
                {
                    eventType = unserializeAddressLine2(Level, xpp, addr);
                }
                else if (tagname.equals("City"))
                {
                    eventType = unserializeAddressCity(Level, xpp, addr);
                }
                else if (tagname.equals("State"))
                {
                    eventType = unserializeAddressState(Level, xpp, addr);
                }
                else if (tagname.equals("Zip"))
                {
                    eventType = unserializeAddressZip(Level, xpp, addr);
                }
                else if (tagname.equals("type"))
                {
                    eventType = unserializeAddresstype(Level, xpp, addr);
                }
            }
            else if (eventType == XmlPullParser.END_TAG)
            {
                Level[0]--;
            }
            if (eventType != XmlPullParser.END_DOCUMENT)
                eventType = xpp.next();
        }
        return eventType;
    }

    private int unserializeAddressLine1(int[] Level, XmlPullParser xpp, AddressInfo addrobj)
            throws XmlPullParserException, IOException
    {
        int eventType = xpp.next();
        int L = Level[0];
        while (eventType != XmlPullParser.END_DOCUMENT && Level[0] >= L)
        {
            if (eventType == XmlPullParser.TEXT)
            {
                addrobj.Line1 = xpp.getText();
            }
            else if (eventType == XmlPullParser.START_TAG)
            {
                Level[0]++;
            }
            else if (eventType == XmlPullParser.END_TAG)
            {
                Level[0]--;
            }
            if (eventType != XmlPullParser.END_DOCUMENT)
                eventType = xpp.next();
        }
        return eventType;
    }

    private int unserializeAddressLine2(int[] Level, XmlPullParser xpp, AddressInfo addrobj)
            throws XmlPullParserException, IOException
    {
        int eventType = xpp.next();
        int L = Level[0];
        while (eventType != XmlPullParser.END_DOCUMENT && Level[0] >= L)
        {
            if (eventType == XmlPullParser.TEXT)
            {
                addrobj.Line2 = xpp.getText();
            }
            else if (eventType == XmlPullParser.START_TAG)
            {
                Level[0]++;
            }
            else if (eventType == XmlPullParser.END_TAG)
            {
                Level[0]--;
            }
            if (eventType != XmlPullParser.END_DOCUMENT)
                eventType = xpp.next();
        }
        return eventType;
    }

    private int unserializeAddressCity(int[] Level, XmlPullParser xpp, AddressInfo addrobj)
            throws XmlPullParserException, IOException
    {
        int eventType = xpp.next();
        int L = Level[0];
        while (eventType != XmlPullParser.END_DOCUMENT && Level[0] >= L)
        {
            if (eventType == XmlPullParser.TEXT)
            {
                addrobj.City = xpp.getText();
            }
            else if (eventType == XmlPullParser.START_TAG)
            {
                Level[0]++;
            }
            else if (eventType == XmlPullParser.END_TAG)
            {
                Level[0]--;
            }
            if (eventType != XmlPullParser.END_DOCUMENT)
                eventType = xpp.next();
        }
        return eventType;
    }

    private int unserializeAddressState(int[] Level, XmlPullParser xpp, AddressInfo addrobj)
            throws XmlPullParserException, IOException
    {
        int eventType = xpp.next();
        int L = Level[0];
        while (eventType != XmlPullParser.END_DOCUMENT && Level[0] >= L)
        {
            if (eventType == XmlPullParser.TEXT)
            {
                addrobj.State = xpp.getText();
            }
            else if (eventType == XmlPullParser.START_TAG)
            {
                Level[0]++;
            }
            else if (eventType == XmlPullParser.END_TAG)
            {
                Level[0]--;
            }
            if (eventType != XmlPullParser.END_DOCUMENT)
                eventType = xpp.next();
        }
        return eventType;
    }

    private int unserializeAddressZip(int[] Level, XmlPullParser xpp, AddressInfo addrobj)
            throws XmlPullParserException, IOException
    {
        int eventType = xpp.next();
        int L = Level[0];
        while (eventType != XmlPullParser.END_DOCUMENT && Level[0] >= L)
        {
            if (eventType == XmlPullParser.TEXT)
            {
                String zipstr = xpp.getText();
                if (zipstr != null && zipstr.length() > 0)
                    addrobj.Zip = Integer.parseInt(zipstr);
            }
            else if (eventType == XmlPullParser.START_TAG)
            {
                Level[0]++;
            }
            else if (eventType == XmlPullParser.END_TAG)
            {
                Level[0]--;
            }
            if (eventType != XmlPullParser.END_DOCUMENT)
                eventType = xpp.next();
        }
        return eventType;
    }

    private int unserializeAddresstype(int[] Level, XmlPullParser xpp, AddressInfo addrobj)
            throws XmlPullParserException, IOException
    {
        int eventType = xpp.next();
        int L = Level[0];
        while (eventType != XmlPullParser.END_DOCUMENT && Level[0] >= L)
        {
            if (eventType == XmlPullParser.TEXT)
            {
                String typestr = xpp.getText();
                if (typestr != null && typestr.length() > 0)
                {
                    if (typestr.equals("Home"))
                        addrobj.type = AddressType.HOME;
                    else if (typestr.equals("Work"))
                        addrobj.type = AddressType.WORK;
                    else if (typestr.equals("Hotel"))
                        addrobj.type = AddressType.HOTEL;
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
            if (eventType != XmlPullParser.END_DOCUMENT)
                eventType = xpp.next();
        }
        return eventType;
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
            respstr.append("</Name>\n");
        }
        if (resp.Addresses != null && resp.Addresses.size() > 0)
        {
            respstr.append("<Addresses>");
            for (int i = 0; i < resp.Addresses.size(); i++)
            {
                AddressInfo addr = resp.Addresses.get(i);
                respstr.append("<Address>");
                if (addr.Line1 != null)
                    respstr.append("<Line1>").append(addr.Line1).append("</Line1>\n");
                if (addr.Line2 != null)
                    respstr.append("<Line2>").append(addr.Line2).append("</Line2>\n");
                if (addr.City != null)
                    respstr.append("<City>").append(addr.City).append("</City>\n");
                if (addr.State != null)
                    respstr.append("<State>").append(addr.State).append("</State>\n");
                respstr.append("<Zip>").append(addr.Zip).append("</Zip>\n");
                respstr.append("<type>").append(addr.type).append("</type>\n");
                respstr.append("</Address>\n");
            }
            respstr.append("</Addresses>\n");
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
            for (int i = 0; i < 10; i++)
                resp = service.JavaEchoPersonInfo(null, req);
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

    private static String readFile(String fpath) throws IOException
    {
        InputStream inputStream = new FileInputStream(fpath);
        long fileSize = new File(fpath).length();
        byte[] allBytes = new byte[(int) fileSize];
        inputStream.read(allBytes);
        return new String(allBytes);
    }
}
