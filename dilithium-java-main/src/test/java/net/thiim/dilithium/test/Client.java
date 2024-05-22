package net.thiim.dilithium.test;
import net.thiim.dilithium.interfaces.DilithiumParameterSpec;
import java.security.KeyPair;
import java.security.KeyPairGenerator;
import java.security.SecureRandom;
import java.security.Signature;


public class Client {

    public void signverify() throws Exception {
        SecureRandom sr = new SecureRandom();
        KeyPairGenerator kpg = KeyPairGenerator.getInstance("Dilithium");
        kpg.initialize(DilithiumParameterSpec.LEVEL2, sr);			
        KeyPair kp = kpg.generateKeyPair();

        Signature sig = Signature.getInstance("Dilithium");
        sig.initSign(kp.getPrivate());
        sig.update("Joy!".getBytes());
        byte[] signature = sig.sign();

        Signature sigv = Signature.getInstance("Dilithium");
        sigv.initVerify(kp.getPublic());
        sigv.update("Joy!".getBytes());
        boolean b = sig.verify(signature);
        System.out.println(b);
        System.out.println("SSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSS");

        try {
            
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
}