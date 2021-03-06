#include "stdafx.h"
#include "CppUnitTest.h"
#include "TextBlock.h"
#include <time.h>
#include <Windows.h>
#include <StrSafe.h>
#include "SharedAdaptiveCard.h"
#include "BaseCardElement.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace AdaptiveCards;
using namespace std;

namespace AdaptiveCardsSharedModelUnitTest
{
    TEST_CLASS(GatherImagesTest)
    {
    private:
        void ValidateResourceInformation(
            std::vector<RemoteResourceInformation>& expectedValues,
            std::vector<RemoteResourceInformation>& resourceInfos)
        {
            Assert::AreEqual(expectedValues.size(), resourceInfos.size());

            for (auto expectedValue : expectedValues)
            {
                bool found = false;
                for (auto resourceInfo : resourceInfos)
                {
                    if (resourceInfo.url == expectedValue.url &&
                        resourceInfo.resourceType == expectedValue.resourceType &&
                        resourceInfo.mimeType == expectedValue.mimeType)
                    {
                        found = true;
                    }
                }

                if (!found)
                {
                    Assert::Fail(L"Expected resource information not found");
                }
            }
        }

    public:
        TEST_METHOD(CanGatherImages)
        {
            // Expected images to find in the card
            std::vector<RemoteResourceInformation> expectedValues = {
                {"BackgroundImage.png", CardElementType::Image, ""},
                { "Image.png", CardElementType::Image, "" },
                { "ImageSet.Image1.png", CardElementType::Image, "" },
                { "ImageSet.Image2.png", CardElementType::Image, "" },
                { "Container.Image1.png", CardElementType::Image, "" },
                { "Container.Image2.png", CardElementType::Image, "" },
                { "ColumnSet.Column1.Image.png", CardElementType::Image, "" },
                { "ColumnSet.Column2.Image.png", CardElementType::Image, "" },
                { "ShowCard.Image.png", CardElementType::Image, "" },
                { "Media.Poster.png", CardElementType::Image, "" },
                { "Media1.mp4", CardElementType::Media, "video/mp4" },
                { "Media2.ogg", CardElementType::Media, "video/ogg" },
            };

            // Test card containing all supported image locations
            std::string testJsonString = 
            "{\
                \"$schema\":\"http://adaptivecards.io/schemas/adaptive-card.json\",\
                \"type\": \"AdaptiveCard\",\
                \"version\": \"1.0\",\
                \"backgroundImage\": \"BackgroundImage.png\",\
                \"body\": [\
                    {\
                        \"type\": \"Image\",\
                        \"url\": \"Image.png\"\
                    },\
                    {\
                        \"type\": \"Media\",\
                        \"poster\": \"Media.Poster.png\",\
                        \"sources\" : [ \
                            {\
                                \"mimeType\": \"video/mp4\",\
                                \"url\" : \"Media1.mp4\"\
                            },\
                            {\
                                \"mimeType\": \"video/ogg\",\
                                \"url\" : \"Media2.ogg\"\
                            }\
                        ]\
                    },\
                    {\
                        \"type\": \"ImageSet\",\
			            \"images\": \
                        [ \
                            {\
                                \"type\": \"Image\", \
                                \"url\" : \"ImageSet.Image1.png\" \
                            }, \
                            {\
                                \"type\": \"Image\", \
                                \"url\" : \"ImageSet.Image2.png\" \
                            } \
                        ] \
                    },\
                    {\
                        \"type\": \"Container\",\
			            \"items\": \
                        [ \
                            {\
                                \"type\": \"Image\", \
                                \"url\" : \"Container.Image1.png\" \
                            }, \
                            {\
                                \"type\": \"Image\", \
                                \"url\" : \"Container.Image2.png\" \
                            } \
                        ] \
                    },\
                    {\
                        \"type\": \"ColumnSet\",\
			            \"columns\": \
                        [ \
                            { \
                                \"type\": \"Column\",\
			                    \"items\": \
                                [ \
                                    {\
                                        \"type\": \"Image\", \
                                        \"url\" : \"ColumnSet.Column1.Image.png\" \
                                    } \
                                ] \
                            }, \
                            { \
                                \"type\": \"Column\",\
			                    \"items\": \
                                [ \
                                    {\
                                        \"type\": \"Image\", \
                                        \"url\" : \"ColumnSet.Column2.Image.png\" \
                                    } \
                                ] \
                            } \
                       ] \
                    }\
                ],\
	            \"actions\": \
                [ \
                    { \
                        \"type\": \"Action.ShowCard\", \
                        \"title\" : \"Action.ShowCard\", \
                        \"card\" : \
                        { \
                            \"type\": \"AdaptiveCard\", \
                            \"body\" : \
                            [ \
                                { \
                                    \"type\": \"Image\", \
                                    \"url\" : \"ShowCard.Image.png\" \
                                } \
                            ] \
                        } \
                    } \
                ] \
            }";

            // Parse the card and get the image uris
            auto resourceInformation = AdaptiveCard::DeserializeFromString(testJsonString, "1.0")->GetAdaptiveCard()->GetResourceInformation();
            ValidateResourceInformation(expectedValues, resourceInformation);
        }

        TEST_METHOD(CanGatherCustomImages)
        {
            // Expected images to find in the card
            std::vector<RemoteResourceInformation> expectedValues = {
                { "BackgroundImage.png", CardElementType::Image, "" },
                { "Image.png", CardElementType::Image, "" },
                { "Custom.png", CardElementType::Image, "" },
                { "Action.Custom.png", CardElementType::Image, "" },
            };

            // Test card containing custom element and action with images
            std::string testJsonString =
                "{\
                \"$schema\":\"http://adaptivecards.io/schemas/adaptive-card.json\",\
                \"type\": \"AdaptiveCard\",\
                \"version\": \"1.0\",\
                \"backgroundImage\": \"BackgroundImage.png\",\
                \"body\": [\
                    {\
                        \"type\": \"Image\",\
                        \"url\": \"Image.png\"\
                    },\
                    {\
                        \"type\": \"CustomTypeWithImage\",\
                        \"customImageProperty\": \"Custom.png\"\
                    }\
                ], \
                \"actions\" : \
                [ \
                    {\
                        \"type\": \"CustomActionWithImage\",\
                        \"customImageProperty\": \"Action.Custom.png\"\
                    }\
                ] \
            }";

            // Define custom type. This implements both element and action for convenience
            class TestCustomElement : public BaseCardElement, public BaseActionElement
            {
                public:
                    TestCustomElement(
                        const Json::Value& value) :
                        BaseCardElement(AdaptiveCards::CardElementType::Custom),
                        BaseActionElement(AdaptiveCards::ActionType::Custom)
                    {
                        m_customImage = value.get("customImageProperty", Json::Value()).asString();
                    }

                    virtual void GetResourceInformation(std::vector<RemoteResourceInformation>& resourceUris) override
                    {
                        RemoteResourceInformation resourceInfo;
                        resourceInfo.url = m_customImage;
                        resourceInfo.resourceType = CardElementType::Image;
                        resourceUris.push_back(resourceInfo);
                    }

                private:
                    std::string m_customImage;
            };

            // Define custom element parser
            class TestCustomParser : public BaseCardElementParser
            {
            public:
                virtual std::shared_ptr<BaseCardElement> Deserialize(
                    std::shared_ptr<AdaptiveCards::ElementParserRegistration> elementParserRegistration,
                    std::shared_ptr<AdaptiveCards::ActionParserRegistration> actionParserRegistration,
                    std::vector<std::shared_ptr<AdaptiveCards::AdaptiveCardParseWarning>>& warnings,
                    const Json::Value& value) override
                {
                    return std::make_shared<TestCustomElement>(value);
                }
            };

            // Define custom action parser
            class TestCustomActionParser : public ActionElementParser
            {
            public:
                virtual std::shared_ptr<BaseActionElement> Deserialize(
                    std::shared_ptr<AdaptiveCards::ElementParserRegistration> elementParserRegistration,
                    std::shared_ptr<AdaptiveCards::ActionParserRegistration> actionParserRegistration,
                    std::vector<std::shared_ptr<AdaptiveCards::AdaptiveCardParseWarning>>& warnings,
                    const Json::Value& value) override
                {
                    return std::make_shared<TestCustomElement>(value);
                }
            };

            // Register the custom parsers
            auto elementRegistration = std::make_shared<ElementParserRegistration>();
            elementRegistration->AddParser("CustomTypeWithImage", std::make_shared<TestCustomParser>());

            auto actionRegistration = std::make_shared<ActionParserRegistration>();
            actionRegistration->AddParser("CustomActionWithImage", std::make_shared<TestCustomActionParser>());

            // Parse the card and get the image uris
            auto resourceInformation = AdaptiveCard::DeserializeFromString(testJsonString, "1.0", elementRegistration, actionRegistration)->GetAdaptiveCard()->GetResourceInformation();
            ValidateResourceInformation(expectedValues, resourceInformation);
        }

        TEST_METHOD(CanGatherImagesInActions)
        {
            // Expected images to find in the card
            std::vector<RemoteResourceInformation> expectedValues = {
                { "BackgroundImage.png", CardElementType::Image, "" },
                { "Image.png", CardElementType::Image, "" },
                { "SubmitAction.png", CardElementType::Image, "" },
                { "OpenUrl.png", CardElementType::Image, "" },
            };

            // Test card containing custom element and action with images
            std::string testJsonString =
                "{\
                \"$schema\":\"http://adaptivecards.io/schemas/adaptive-card.json\",\
                \"type\": \"AdaptiveCard\",\
                \"version\": \"1.0\",\
                \"backgroundImage\": \"BackgroundImage.png\",\
                \"body\": [\
                    {\
                        \"type\": \"Image\",\
                        \"url\": \"Image.png\"\
                    },\
                    {\
                        \"type\": \"CustomTypeWithImage\",\
                        \"customImageProperty\": \"Custom.png\"\
                    }\
                ], \
                \"actions\" : \
                [ \
                    {\
                        \"type\": \"Action.Submit\",\
                        \"title\": \"Snooze\",\
                        \"iconUrl\": \"SubmitAction.png\"\
                    },\
                    {\
                        \"type\": \"Action.Submit\",\
                        \"title\": \"Dismiss\"\
                    },\
                    {\
                        \"type\": \"Action.OpenUrl\",\
                        \"title\" : \"Action.OpenUrl\",\
                        \"iconUrl\": \"OpenUrl.png\",\
                        \"url\" : \"http://adaptivecards.io\"\
                    }\
                ] \
            }";

            // Parse the card and get the image uris
            auto resourceInformation = AdaptiveCard::DeserializeFromString(testJsonString, "1.0")->GetAdaptiveCard()->GetResourceInformation();
            ValidateResourceInformation(expectedValues, resourceInformation);
        }
    };
}
