USE WebVisu;

INSERT INTO GuiElementTypes (`type`, description) VALUES
	('button','just a stupid Button'),
	('textInput', 'input Field for inserting a text'),
	('label', 'label for showing constant Text');
	

Insert INTO Pages (ParentID, title, description) values
	(NULL,'ROOT_PAGE', 'the landing Page of the Visu'),
	(1,'SUB_PAGE', 'first sub page'),
	(1,'SUB_PAGE2', 'second sub page');


INSERT INTO DataTypes (type) values
	('Bool'),
	('Int8'),
	('Int16'),
	('Int32'),
	('Int64'),
	('UInt8'),
	('UInt16'),
	('UInt32'),
	('UInt64'),
	('Float'),
	('Double'),
	('String'),
	('DateTime');
INSERT INTO GuiElementDataNodeTemplates (writePermission, type, qualifiedName, defaultValue, description) values
	(false, 'Bool','buttonState','0','DataNode which holds the value of the Button a Button'),
	(true, 'UInt8', 'colorSelector','0','DataNode to mux different colors to display a state'),
	(true, 'String', 'text', 'ButtonText', 'text which is be displayed on the Button'),
	(true, 'String', 'text', '', 'text which was typed into and displayed on the Text input field'),
	(true, 'UInt8', 'textSelector','0','DataNode to mux different texts to display');
INSERT INTO GuiElementParamTemplates (type, qualifiedName, defaultValue, description) values
	('String', 'colorEnum', '["red","green","blue","#FF00FF"]','JSON Array with the colors that are selectable with the "colorSelector DataNode"'),
	('UInt16', 'sizeX', '100', 'maximum Size of the GuiElement in X'),
	('UInt16', 'sizeY', '50', 'maximum Size of the GuiElement in Y'),
	('UInt16', 'posX', '0', 'position of the GuiElement in  X counted from the left side in %'),
	('UInt16', 'posY', '0', 'position of the GuiElement in  Y counted from the top of the Page in px'),
	('String', 'color', 'blue','primary color'),
	('String', 'textColor', 'white','the color of the text'),
	('String', 'textEnum', '["first Text","second text", "third text"]','JSON Array with the different texts which can selected with the "textSelector DataNode"');
INSERT INTO GuiElementTypesDataNodesRel (GuiElementTypeID, DataNodeTemplateID) values
	(1, 1),
	(1, 2),
	(1, 3),
	(2, 2),
	(2, 4),
	(3, 5);
INSERT INTO GuiElementTypesParamsRel (GuiElementTypeID, ParamTemplateID) values
	(1, 1),
	(1, 2),
	(1, 3),
	(1, 4),
	(1, 5),
	(2, 1),
	(2, 2),
	(2, 3),
	(2, 4),
	(2, 5),
	(3, 6),
	(3, 7),
	(3, 8),
	(3, 2),
	(3, 3),
	(3, 4),
	(3, 5);
-- CREATE SAMPLEDATA

CALL CreateInstanceOfGuiElement('button', 1, 'der erste Button');
CALL CreateInstanceOfGuiElement('textInput', 1, 'ein Textfeld');
CALL CreateInstanceOfGuiElement('label', 1, 'ein Label');
CALL CreateInstanceOfGuiElement('button', 2, 'der zweite Button');
CALL CreateInstanceOfGuiElement('button', 2, 'der 3. Button');
CALL CreateInstanceOfGuiElement('button', 2, 'der 4. Button');
CALL CreateInstanceOfGuiElement('button', 2, 'der 5. Button');
CALL CreateInstanceOfGuiElement('button', 2, 'der 6. Button');
CALL CreateInstanceOfGuiElement('button', 2, 'der 7. Button');
insert into Credentials (userName, pwHash) VALUES ('user', MD5('testPW'));
insert into Credentials (userName, pwHash) VALUES ('WebVisuSU', MD5('637013'));
