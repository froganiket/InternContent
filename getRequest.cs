using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.Networking;
using UnityEngine.Serialization;
using UnityEngine.InputSystem;
using UnityEngine.XR.Interaction.Toolkit;

[System.Serializable]
public class DataResponse
{
    public int ClosePositionRef;
}

public class getRequest : MonoBehaviour
{
    private const string EndpointURL = "http://172.20.10.6:3000/close"; // Update with your server URL
    public float requestInterval = 0f; // Interval between each GET request
    [SerializeField] private Animator animator;
    private GameObject arm;
    [SerializeField] float value = 0f;
    private UnityWebRequest request;
    private bool increasing = true;

    // public InputActionProperty grabProperty;
    // private XRDirectInteractor armInteractor;


    void Start()
    {
        StartCoroutine(SendDataRequest());
        arm = GameObject.Find("Aniket robot  Variant 1");
        // armInteractor.hoverEntered.AddListener(armInteractions);
    }

    IEnumerator SendDataRequest()
    {
        while (true)
        {
            request = UnityWebRequest.Get(EndpointURL);

            yield return request.SendWebRequest();

            if (request.result != UnityWebRequest.Result.Success)
            {
                Debug.LogError("Failed to receive data from server: " + request.error);
            }
            else
            {
                string receivedData = request.downloadHandler.text;
                Debug.Log("Received data: " + receivedData);

                // Parse the JSON data
                DataResponse responseData = JsonUtility.FromJson<DataResponse>(receivedData);

                if (responseData != null && responseData.ClosePositionRef == 1)
                {
                    if (increasing)
                    {
                        // Increase value until it reaches 1
                        while (value < 1f)
                        {
                            value += 0.1f;
                            animator?.SetFloat("Blend", value);
                            yield return new WaitForSeconds(0.1f);
                        }

                        increasing = false;
                    }
                    else
                    {
                        // Decrease value until it reaches 0
                        while (value > 0f)
                        {
                            value -= 0.1f;
                            animator?.SetFloat("Blend", value);
                            yield return new WaitForSeconds(0.1f);
                        }

                        increasing = true;
                    }
                }
            }

            request.Dispose(); // Manually dispose the UnityWebRequest object

            yield return new WaitForSeconds(requestInterval);
        }
    }

    void Update()
    {
        if (value < 0)
        {
            value = 0;
        }
        else if (value > 1)
        {
            value = 1;
        }
        // grabProperty.action.ReadValue<float>();
        animator?.SetFloat("Blend", value);
    }

    // void armInteractions(HoverEnterEventArgs hoverArgs)
    // {
    //     while (true)
    //     {
    //         if (value > 0.5)
    //         {
    //             armInteractor.startingSelectedInteractable = (XRBaseInteractable)hoverArgs.interactableObject;
    //         }
    //         else
    //         {
    //             armInteractor.startingSelectedInteractable = null;
    //         }
    //     }
    // }
}
